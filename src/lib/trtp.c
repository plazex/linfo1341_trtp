#include "trtp.h"
#include "logger.h"

#include <time.h>
//need to install:sudo apt-get install libz-dev
#include <zlib.h>

#define TIMEOUT 1000 //start with static timeout
#define RCV_WINDOW_SIZE 4 //size of the receiver window

int trtp_send_response(UdpSocket *udpSocket, char *buf, TrtpFrame *frame)
{
    frame->tr = 0;
    frame->timestamp = (int)clock();
    frame->length = 0;

    memset(buf, 0, HEADER_LENGTH);
    encodeFrame(buf, frame);
    return udp_send(buf, HEADER_LENGTH, udpSocket);
}

int trtp_send_data(FILE *pfile, UdpSocket *udpSocket, char *buf, TrtpFrame *frame) {
    int length, total_length;
    char *payload = buf + HEADER_LENGTH;

    memset(buf, 0, MAX_FRAME);

    fseek(pfile, frame->seqnum * MAX_PAYLOAD, SEEK_SET);
    length = fread(payload, 1, MAX_PAYLOAD, pfile);
    total_length = HEADER_LENGTH + length + 4;

    frame->type = PTYPE_DATA;
    frame->tr = 0;
    frame->length = length;
    frame->crc2 = crc32(0L, payload, length);

    encodeFrame(buf, frame);
    return udp_send(buf, total_length, udpSocket);
}

int trtp_send(FILE *pfile, UdpSocket *udpSocket) {   
    int data_sent = 0, ack_received = 0, nack_received = 0;
    int min_rtt, max_rtt = 0, packet_retransmitted = 0;

    int smoothed_rtt = TIMEOUT; // using smoothed RTT, initialisation to default timeout
    float alpha = 0.125;

    int pool_result, pool_timer, length, nb_frames, seqnum;
    int send_base = 0, fd_count = 1; 
    int window_size = 1; // taille de fenetre initiale d'envoie
    int timestamps[MAX_WINDOW_SIZE] = {0};
    size_t payload_size;
    char buf[MAX_FRAME];
    struct pollfd *pfds = malloc(sizeof(*pfds));
    TrtpFrame frame;

    pfds[0].fd = udpSocket->sock;
    pfds[0].events = POLLIN;
    min_rtt = INT_MAX; // initialize to max int

    fseek(pfile, 0L, SEEK_END);
    payload_size = ftell(pfile);
    nb_frames = (payload_size / MAX_PAYLOAD) + (payload_size % MAX_PAYLOAD != 0);
    fseek(pfile, 0L, SEEK_SET);
    
    for(;;) { // Main loop
        pool_timer = smoothed_rtt;
        if(send_base > nb_frames) { //end of sending
            break;
        }
        else if(send_base == nb_frames) {
            int new_timer = (int)clock();

            timestamps[send_base % window_size] = new_timer;
            frame.timestamp = new_timer;
            frame.seqnum = send_base;
            frame.type = PTYPE_DATA;
            frame.window = window_size;
            if(trtp_send_response(udpSocket, buf, &frame) < 0) {
                return -1;
            }
            data_sent++; // incr data sent for the last frame
        }
        else {
            for(seqnum = send_base; seqnum < nb_frames && seqnum < send_base + window_size; seqnum++) {
                int old_timer = timestamps[seqnum % window_size];
                int new_timer = (int)clock();
                int last_time = new_timer - old_timer;

                if(last_time > smoothed_rtt) {
                    timestamps[seqnum % window_size] = new_timer;
                    frame.timestamp = new_timer;
                    frame.seqnum = seqnum;
                    frame.window = window_size;
                    if(trtp_send_data(pfile, udpSocket, buf, &frame) < 0) {
                        return -1;
                    }
                    data_sent++; // incr data sent
                    if(old_timer != 0) {
                        packet_retransmitted++; // incr packet retransmitted due to timeout
                    }
                }
                else if(old_timer >= 0 && last_time < pool_timer) {
                    pool_timer = last_time; //keep the min time
                }
            }
        }
        
        pool_result = poll(pfds, fd_count, pool_timer);

        int rtt = (int)clock() - timestamps[send_base % window_size];
        if(rtt < min_rtt) min_rtt = rtt;
        if(rtt > max_rtt) max_rtt = rtt;

        smoothed_rtt = ((1 - alpha) * smoothed_rtt) + (alpha * rtt); // update the smoothed RTT
        
        if(pool_result > 0 && pfds[0].revents & POLLIN) { // If ack or nack is available
            memset(buf, 0, HEADER_LENGTH);
            if (udp_receive(buf, HEADER_LENGTH, udpSocket) < 0) {
                return -1;
            }
            decodeFrame(buf, &frame);
            if(frame.tr != 0) { //truncated
                continue;
            }
            
            if(crc32(0L, buf, HEADER_LENGTH - 4) != frame.crc1) { //wrong crc1
                continue;
            }
            
            if(send_base == 0) { //set only one time
                window_size = frame.window; // update the window size 
            }

            if(frame.seqnum < send_base || frame.seqnum >= send_base + window_size) { //out of the window
                continue;
            }

            if(frame.type == PTYPE_ACK) {
                ack_received++; // incr ack received
                // cumulative frame: move the send_base to the requested frame seqnum
                while (frame.seqnum > send_base) {
                    timestamps[send_base++ % window_size] = 0;
                }
            } 
            else if(frame.type == PTYPE_NACK) {
                nack_received++; // incr nack received

                int new_timer = (int)clock();
                timestamps[frame.seqnum % window_size] = new_timer;
                frame.timestamp = new_timer;
                frame.window = window_size;
                if(trtp_send_data(pfile, udpSocket, buf, &frame) < 0) {
                    return -1;
                }
                packet_retransmitted++; // incr packet retransmitted due to nack
            }
        } // in case of timeout pool_result == 0, nothing to do
    }

    printStat(DATA_SENT, data_sent);
    printStat(ACK_RECEIVED, ack_received);
    printStat(NACK_RECEIVED, nack_received);
    printStat(MIN_RTT, min_rtt);
    printStat(MAX_RTT, max_rtt);
    printStat(PACKET_RETRANSMITTED, packet_retransmitted);

    return 1;
}

int trtp_listen(UdpSocket *udpSocket) {  
    int data_received = 0, data_truncated_received = 0, ack_sent = 0, 
        nack_sent = 0, packet_ignored = 0;
    int packet_duplicated = 0, packet_recovered = 0;

    int index;
    int send_base = 0, fd_count = 1; 
    int window_size = RCV_WINDOW_SIZE; // taille de fenetre de reception
    char buf[MAX_FRAME]; //rcv buffer
    char rcv_window[RCV_WINDOW_SIZE] = {0}; //rcv window
    char rcv_data[RCV_WINDOW_SIZE * MAX_PAYLOAD]; // data memory
    struct pollfd *pfds = malloc(sizeof(*pfds));
    TrtpFrame frame;

    pfds[0].fd = udpSocket->sock;
    pfds[0].events = POLLIN;
    
    for(;;) { // Main loop
        if (poll(pfds, fd_count, -1) < 1) {
            perror("pollserver failed");
            return -1;
        }
        if (pfds[0].revents & POLLIN) { // If data available
            memset(buf, 0, MAX_FRAME);
            if (udp_receive(buf, MAX_FRAME, udpSocket) < 0) {
                return -1;
            }
            decodeFrame(buf, &frame);
            if(frame.type != PTYPE_DATA) { //wrong type
                continue;
            }
            data_received++; // incr received data
            if(frame.tr != 0) { //truncated: send nack
                data_truncated_received++; // incr received truncated data
                frame.type = PTYPE_NACK;
                frame.window = window_size;
                if(trtp_send_response(udpSocket, buf, &frame) < 0) {
                    return -1;
                }
                nack_sent++; // incr sent nack
                continue;
            }
            if(crc32(0L, buf, HEADER_LENGTH - 4) != frame.crc1) { //wrong crc1: ignored
                packet_ignored++; // incr ignored packet due to wrong crc1
                continue;
            }
            if(crc32(0L, buf + HEADER_LENGTH, frame.length) != frame.crc2) { //wrong crc2: ignored
                packet_ignored++; // incr ignored packet due to wrong crc2
                continue;
            }
            if(frame.seqnum < send_base || frame.seqnum >= send_base + window_size) { //out of the window
                packet_ignored++; // incr ignored packet due to out of the window
                continue;
            }
            index = frame.seqnum % window_size;
            if(rcv_window[index] == 1) { //already received
                packet_ignored++; // incr ignored packet due to duplicate
                packet_duplicated++; // incr duplicated packet
                continue;
            }
            if(frame.length == 0) { //all frames have been received
                frame.seqnum++;
                frame.type = PTYPE_ACK;
                frame.window = window_size;
                if(trtp_send_response(udpSocket, buf, &frame) < 0) {
                    return -1;
                }
                ack_sent++; // incr sent ack
                break;
            }

            rcv_window[index] = 1;
            if(frame.length < MAX_PAYLOAD) { //last frame
                memset(rcv_data + (index * MAX_PAYLOAD), 0, MAX_PAYLOAD);
            }
            memcpy(rcv_data + (index * MAX_PAYLOAD), buf + HEADER_LENGTH, frame.length);

            if(frame.seqnum == send_base) { // in order
                while (rcv_window[index] == 1) {
                    printf("%.*s", frame.length, buf + HEADER_LENGTH);
                    rcv_window[index] = 0;
                    send_base++;
                    index = send_base % window_size;
                }
            }

            frame.seqnum = send_base;
            frame.type = PTYPE_ACK;
            frame.window = window_size;
            if(trtp_send_response(udpSocket, buf, &frame) < 0) {
                return -1;
            }
            ack_sent++; // incr sent ack
        }
    }

    printStat(DATA_RECEIVED, data_received);
    printStat(DATA_TRUNCATED_RECEIVED, data_truncated_received);
    printStat(ACK_SENT, ack_sent);
    printStat(NACK_SENT, nack_sent);
    printStat(PACKET_IGNORED, packet_ignored);
    printStat(PACKET_DUPLICATED, packet_duplicated);
    printStat(PACKET_RECOVERED, packet_recovered);

    return 1;
}
