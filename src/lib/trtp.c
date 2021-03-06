#include "trtp.h"
#include "logger.h"
#include "command.h"

#include <time.h>
//need to install:sudo apt-get install libz-dev
#include <zlib.h>

#define TIMEOUT 1000 //start with static timeout
#define MAX_TIMEOUT 1000 //maximum timeout
#define FEC_CUMUL 4 //nb data frame to form a fec frame
#define RCV_WINDOW_SIZE 4 //size of the receiver window

int apply_loss() {
    if(cmd_loss) {
        return (rand() % 100) < cmd_loss ? 1 : 0;
    }
    return 0;
}

int apply_truncate() {
    if(cmd_truncated) {
        int rd = rand() % 100;
        return (rand() % 100) < cmd_truncated ? 1 : 0;
    }
    return 0;
}

int trtp_send_response(UdpSocket *udpSocket, char *buf, TrtpFrame *frame)
{
    frame->tr = apply_truncate();
    frame->timestamp = (int)clock();
    frame->length = 0;

    memset(buf, 0, HEADER_LENGTH);
    encodeFrame(buf, frame);

    if(apply_loss()) {
        return 1;
    }

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
    frame->tr = apply_truncate();
    frame->length = length;
    frame->crc2 = crc32(0L, payload, length);

    encodeFrame(buf, frame);

    if(apply_loss()) {
        return 1;
    }

    return udp_send(buf, total_length, udpSocket);
}

int trtp_send_fec(UdpSocket *udpSocket, char *fec_buf, char *buf, TrtpFrame *frame) {
    char *payload = buf + HEADER_LENGTH;

    memset(buf, 0, MAX_FRAME);
    memcpy(payload, fec_buf, MAX_PAYLOAD);

    frame->type = PTYPE_FEC;
    frame->tr = apply_truncate();
    frame->length = MAX_PAYLOAD; // same length for all fec and carried data
    frame->crc2 = crc32(0L, payload, MAX_PAYLOAD);

    encodeFrame(buf, frame);

    if(apply_loss()) {
        return 1;
    }

    return udp_send(buf, MAX_FRAME, udpSocket);
}

int trtp_send(FILE *pfile, UdpSocket *udpSocket) {   
    int data_sent = 0, fec_sent = 0, ack_received = 0, nack_received = 0;
    int min_rtt, max_rtt = 0, packet_retransmitted = 0, packet_ignored = 0;

    int smoothed_rtt = TIMEOUT; // using smoothed RTT, initialisation to default timeout
    int cumul_timer = 0;
    float alpha = 0.125;

    int pool_result, pool_timer, length, nb_frames, seqnum;
    int send_base = 0, fd_count = 1, fec_count = 0; 
    int window_size = 1; // initial window size
    int *timestamps = malloc(sizeof(int));
    
    size_t payload_size;
    char buf[MAX_FRAME];
    char fec_buf[MAX_PAYLOAD];
    struct pollfd *pfds = malloc(sizeof(*pfds));
    TrtpFrame frame;

    pfds[0].fd = udpSocket->sock;
    pfds[0].events = POLLIN;
    min_rtt = INT_MAX; // initialize to max int
    srand(time(NULL));
    timestamps[0] = 0;

    fseek(pfile, 0L, SEEK_END);
    payload_size = ftell(pfile);
    nb_frames = (payload_size / MAX_PAYLOAD) + (payload_size % MAX_PAYLOAD != 0);
    fseek(pfile, 0L, SEEK_SET);

    int global_timer = (int)clock(); // keep the timer without the computation time
    
    for(;;) { // Main loop
        pool_timer = smoothed_rtt;
        if(send_base > nb_frames) { //end of sending
            break;
        }
        else if(cumul_timer > MAX_TIMEOUT) {
            fprintf(stderr, "SENDER: come out due to max timeout[%d]\n", cumul_timer);
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
            fprintf(stderr, "SENDER: send last packet\n");
            data_sent++; // incr data sent for the last frame
        }
        else {
            for(seqnum = send_base; seqnum < nb_frames && seqnum < send_base + window_size; seqnum++) {
                int old_timer = timestamps[seqnum % window_size];
                int new_timer = (int)clock();
                int last_time = global_timer - old_timer;

                if(last_time > smoothed_rtt) {
                    timestamps[seqnum % window_size] = new_timer;
                    frame.timestamp = new_timer;
                    frame.seqnum = seqnum;
                    frame.window = window_size;
                    if(trtp_send_data(pfile, udpSocket, buf, &frame) < 0) {
                        return -1;
                    }
                    fprintf(stderr, "SENDER: send data seqnum=%d\n", frame.seqnum);
                    data_sent++; // incr data sent
                    if(old_timer != 0) {
                        packet_retransmitted++; // incr packet retransmitted due to timeout
                    }
                    if(cmd_fec) {
                        xor(fec_buf, buf + HEADER_LENGTH, MAX_PAYLOAD, frame.length);
                        fec_count++;
                        if(fec_count >= FEC_CUMUL) {
                            if(trtp_send_fec(udpSocket, fec_buf, buf, &frame) < 0) {
                                return -1;
                            }
                            fec_sent++; // incr fec sent
                            fec_count = 0;
                            memset(fec_buf, 0, MAX_PAYLOAD);
                        }
                    }
                }
                else if(old_timer >= 0 && last_time < pool_timer) {
                    pool_timer = last_time; //keep the min time
                }
            }
        }
        int begin = (int)clock();
        pool_result = poll(pfds, fd_count, pool_timer);
        global_timer = (int)clock();
        
        if(pool_result > 0 && pfds[0].revents & POLLIN) { // If ack or nack is available
            cumul_timer = 0; // reset cumul timer
            memset(buf, 0, HEADER_LENGTH);
            if (udp_receive(buf, HEADER_LENGTH, udpSocket) < 0) {
                return -1;
            }
            decodeFrame(buf, &frame);
            if(frame.tr != 0) { //truncated
                packet_ignored++; // ignored packet -> truncated
                continue;
            }
            
            if(crc32(0L, buf, HEADER_LENGTH - 4) != frame.crc1) { //wrong crc1
                packet_ignored++; // ignored packet -> wrong crc1
                continue;
            }
            
            if(send_base == 0) { //set only one time
                window_size = frame.window; // update the window size 
                timestamps = malloc(window_size * sizeof(int));
                memset(timestamps, 0, window_size);
            }

            if(frame.seqnum < send_base || frame.seqnum > send_base + window_size) { //out of the window
                packet_ignored++; // ignored packet -> out of the window
                continue;
            }

            if(frame.type == PTYPE_ACK) {
                int rtt = global_timer - timestamps[send_base % window_size];
                if(rtt < min_rtt) min_rtt = rtt;
                if(rtt > max_rtt) max_rtt = rtt;

                //stop using smoothed rtt
                //smoothed_rtt = ((1 - alpha) * smoothed_rtt) + (alpha * rtt); // update the smoothed RTT

                ack_received++; // incr ack received
                // cumulative frame: move the send_base to the requested frame seqnum
                while (frame.seqnum > send_base) {
                    timestamps[send_base++ % window_size] = 0;
                }
                fprintf(stderr, "SENDER: ack received[%d] seqnum=%d send_base=%d\n", ack_received, frame.seqnum, send_base);
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
        } 
        else { // in case of timeout pool_result == 0, increase cumul timer
            cumul_timer += global_timer - begin;
            fprintf(stderr, "SENDER: timer[%d]\n", cumul_timer);
        }
    }

    printStat(DATA_SENT, data_sent);
    printStat(FEC_SENT, fec_sent);
    printStat(ACK_RECEIVED, ack_received);
    printStat(NACK_RECEIVED, nack_received);
    printStat(MIN_RTT, min_rtt);
    printStat(MAX_RTT, max_rtt);
    printStat(PACKET_IGNORED, packet_ignored);
    printStat(PACKET_RETRANSMITTED, packet_retransmitted);

    free(timestamps);
    free(pfds);
    udp_close(udpSocket);

    return 1;
}

int trtp_listen(UdpSocket *udpSocket) {  
    int data_received = 0, fec_received = 0, data_truncated_received = 0, 
        ack_sent = 0, nack_sent = 0, packet_ignored = 0;
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
    srand(time(NULL));
    
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
            if(frame.type == PTYPE_FEC) { //fec type
                fec_received++; // incr received fec
                if(frame.tr != 0) {
                    packet_ignored++; // incr ignored packet due to truncated fec
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
                if(frame.seqnum != send_base + FEC_CUMUL - 1 || FEC_CUMUL > window_size) { 
                    packet_ignored++; // incr ignored packet due to fetch data not possible
                    continue;
                }
                int not_fetched = 0, fetch_index = 0, new_seqnum = 0;
                for(int i = send_base; i <= frame.seqnum; i++) {
                    if(rcv_window[i % window_size] == 0) {
                        not_fetched++;
                        index = i % window_size; // update the data index to fetch
                        new_seqnum = i; // keep also the seqnum
                    }
                }
                if(not_fetched != 1) {
                    // incr ignored packet due to no loss (not_fetched = 0) or too much loss (not_fetch > 1)
                    packet_ignored++; 
                    continue;
                }
                for(int i = send_base; i <= frame.seqnum; i++) {
                    fetch_index = i % window_size;
                    if(rcv_window[fetch_index] == 1) {
                        xor(buf + HEADER_LENGTH, rcv_data + (fetch_index * MAX_PAYLOAD), frame.length, MAX_PAYLOAD);
                    }
                }
                packet_recovered++; // incr recovered packet
                frame.seqnum = new_seqnum;
                fprintf(stderr, "RECV: recovered data[%d] seqnum=%d\n", packet_recovered, frame.seqnum);
            }
            else if(frame.type == PTYPE_DATA) { //data type
                data_received++; // incr received data
                if(frame.tr != 0) { //truncated: send nack
                    data_truncated_received++; // incr received truncated data
                    fprintf(stderr, "RECV: truncated data[%d] seqnum=%d\n", data_truncated_received, frame.seqnum);
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
                if(frame.seqnum < 0 || frame.seqnum >= send_base + window_size) { //out of the window
                    packet_ignored++; // incr ignored packet due to out of the window
                    continue;
                }
                index = frame.seqnum % window_size;
                if(frame.seqnum < send_base || rcv_window[index] == 1) { //already received
                    packet_duplicated++; // incr duplicated packet
                    fprintf(stderr, "RECV: duplicate data[%d] seqnum=%d send_base=%d\n", packet_duplicated, frame.seqnum, send_base);
                    frame.seqnum = send_base;
                    frame.type = PTYPE_ACK;
                    frame.window = window_size;
                    if(trtp_send_response(udpSocket, buf, &frame) < 0) {
                        return -1;
                    }
                    ack_sent++; // incr sent ack
                    continue;
                }
                if(frame.length == 0) { //all frames have been received
                    frame.seqnum++;
                    frame.type = PTYPE_ACK;
                    frame.window = window_size;
                    fprintf(stderr, "RECV: last packet seqnum=%d\n", frame.seqnum);
                    if(trtp_send_response(udpSocket, buf, &frame) < 0) {
                        return -1;
                    }
                    ack_sent++; // incr sent ack
                    break;
                }
            }
            else { // wrong type
                continue;
            }

            fprintf(stderr, "RECV: receive data seqnum %d\n", frame.seqnum);
            rcv_window[index] = 1;
            if(frame.length < MAX_PAYLOAD) { //last frame
                memset(rcv_data + (index * MAX_PAYLOAD), 0, MAX_PAYLOAD);
            }
            memcpy(rcv_data + (index * MAX_PAYLOAD), buf + HEADER_LENGTH, frame.length);

            if(frame.seqnum == send_base) { // in order
                while (rcv_window[index] == 1) {
                    fprintf(stderr, "RECV: write data seqnum %d at index=%d\n", send_base, (index * MAX_PAYLOAD));
                    fwrite(rcv_data + (index * MAX_PAYLOAD), 1, frame.length, stdout);
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
    printStat(FEC_RECEIVED, fec_received);
    printStat(DATA_TRUNCATED_RECEIVED, data_truncated_received);
    printStat(ACK_SENT, ack_sent);
    printStat(NACK_SENT, nack_sent);
    printStat(PACKET_IGNORED, packet_ignored);
    printStat(PACKET_DUPLICATED, packet_duplicated);
    printStat(PACKET_RECOVERED, packet_recovered);

    free(pfds);
    udp_close(udpSocket);

    return 1;
}
