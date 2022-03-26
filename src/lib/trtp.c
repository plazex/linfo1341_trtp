#include "trtp.h"

#include <time.h>
//need to install:sudo apt-get install libz-dev
#include <zlib.h>

#define TIMEOUT 200 //start with static timeout
#define RCV_WINDOW_SIZE 4 //size of the receiver window

int window_size = 1; // taille de fenetre initiale

int trtp_send_response(UdpSocket *udpSocket, char *buf, TrtpFrame *frame, uint8_t type)
{
    frame->type = type;
    frame->tr = 0;
    frame->window = window_size;
    frame->timestamp = (int)time(NULL);
    frame->length = 0;
    frame->crc1 = crc32(0L, (char *)frame, HEADER_LENGTH - 4); //crc32 with frame memory

    memset(buf, 0, HEADER_LENGTH);
    encodeFrame(buf, frame);
    return udp_send(buf, HEADER_LENGTH, udpSocket);
}

int trtp_send_data(FILE *pfile, UdpSocket *udpSocket, char *buf, TrtpFrame *frame) {
    int length, total_length;
    char *payload = buf + HEADER_LENGTH;

    fseek(pfile, frame->seqnum, SEEK_SET);
    length = fread(payload, 1, MAX_PAYLOAD, pfile);
    total_length = HEADER_LENGTH + length + 4;

    frame->type = PTYPE_DATA;
    frame->tr = 0;
    frame->window = window_size;
    frame->length = length;
    frame->crc1 = crc32(0L, (char *)frame, HEADER_LENGTH - 4); //crc32 with frame memory
    frame->crc2 = crc32(0L, payload, length);

    memset(buf, 0, MAX_FRAME);
    encodeFrame(buf, frame);
    return udp_send(buf, total_length, udpSocket);
}

int trtp_send(FILE *pfile, UdpSocket *udpSocket) {   
    int pool_result, pool_timer, length, nb_frames, seqnum;
    int send_base = 0, fd_count = 1; 
    int timestamps[MAX_WINDOW_SIZE] = {0};
    size_t payload_size;
    char buf[MAX_FRAME];
    char *payload = buf + HEADER_LENGTH;
    struct pollfd *pfds = malloc(sizeof(*pfds));
    TrtpFrame frame;

    pfds[0].fd = udpSocket->sock;
    pfds[0].events = POLLIN;

    fseek(pfile, 0L, SEEK_END);
    payload_size = ftell(pfile);
    nb_frames = payload_size / MAX_PAYLOAD;
    fseek(pfile, 0L, SEEK_SET);

    frame.timestamp = payload_size;
    
    for(;;) { // Main loop
        if(send_base >= nb_frames) {
            break;
        }
        pool_timer = TIMEOUT;
        for(seqnum = send_base; seqnum < nb_frames && seqnum < send_base + window_size; seqnum++) {
            int old_timer = timestamps[seqnum % window_size];
            int new_timer = (int)time(NULL);
            int last_time = new_timer - old_timer;

            if(old_timer >= 0 && last_time > TIMEOUT) {
                timestamps[seqnum % window_size] = new_timer;
                frame.seqnum = seqnum;
                if(trtp_send_data(pfile, udpSocket, buf, &frame) < 0) {
                    return -1;
                }
            }
            else if(old_timer >= 0 && last_time < pool_timer) {
                pool_timer = last_time; //keep the min time
            }
        }
        pool_result = poll(pfds, fd_count, pool_timer);

        if (pool_result < 1) {
            perror("pollserver failed");
            return -1;
        }
        else if(pool_result > 0 && pfds[0].revents & POLLIN) { // If ack or nack is available
            memset(buf, 0, HEADER_LENGTH);
            if (udp_receive(buf, HEADER_LENGTH, udpSocket) < 0) {
                return -1;
            }
            decodeFrame(buf, &frame);
            if(frame.tr != 0) { //truncated
                continue;
            }
            if(crc32(0L, (char *)&frame, HEADER_LENGTH - 4) != frame.crc1) { //wrong crc1
                continue;
            }
            if(frame.seqnum < send_base || frame.seqnum >= send_base + window_size) { //out of the window
                continue;
            }
            
            if(send_base == 0) { //set only one time
                window_size = frame.window; // update the window size 
            }
            
            if(frame.type == PTYPE_ACK) {
                timestamps[frame.seqnum % window_size] = -1;
                int count = 0;
                while (timestamps[send_base % window_size] == -1 && count < window_size) {
                    timestamps[send_base % window_size] = 0;
                    send_base++;
                    count++;
                }
            } 
            else if(frame.type == PTYPE_NACK) {
                int new_timer = (int)time(NULL);
                timestamps[frame.seqnum - send_base] = new_timer;
                if(trtp_send_data(pfile, udpSocket, buf, &frame) < 0) {
                    return -1;
                }
            }
        } // in case of timeout pool_result == 0, nothing to do
    }
    return 1;
}

int trtp_listen(UdpSocket *udpSocket) {   
    int total_size;
    int send_base = 0, fd_count = 1, nb_frames = 0; 
    char buf[MAX_FRAME]; //rcv buffer
    char rcv_window[RCV_WINDOW_SIZE] = {0}; //rcv window
    char *result = NULL; // data memory
    struct pollfd *pfds = malloc(sizeof(*pfds));
    TrtpFrame frame;

    pfds[0].fd = udpSocket->sock;
    pfds[0].events = POLLIN;

    window_size = RCV_WINDOW_SIZE;
    
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
            if(frame.tr != 0) { //truncated: send nack
                if(trtp_send_response(udpSocket, buf, &frame, PTYPE_NACK) < 0) {
                    return -1;
                }
                continue;
            }
            if(crc32(0L, (char *)&frame, HEADER_LENGTH - 4) != frame.crc1) { //wrong crc1: ignored
                continue;
            }
            if(frame.seqnum < send_base || frame.seqnum >= send_base + window_size) { //out of the window
                continue;
            }

            if(result == NULL) {
                total_size = frame.timestamp;
                nb_frames = total_size / MAX_PAYLOAD;
                memset(result, 0, total_size);
            }

            memcpy(result + (frame.seqnum * MAX_PAYLOAD), buf + HEADER_LENGTH, frame.length);

            if(frame.seqnum == nb_frames - 1) {
                result[total_size - 1] = '\0';
                printf("%s", result);
                break;
            }

            rcv_window[frame.seqnum % window_size] = 1;
            int count = 0;
            while (rcv_window[send_base % window_size] == 1 && count < window_size) {
                rcv_window[send_base % window_size] = 0;
                send_base++;
                count++;
            }
        }
    }
    return 1;
}
