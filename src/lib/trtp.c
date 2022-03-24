#include "trtp.h"

#include <time.h>
//need to install:sudo apt-get install libz-dev
#include <zlib.h>

#define TIMEOUT 200 //start with static timeout

int window_size = 1; // taille de fenetre initiale

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
    
    for(;;) { // Main loop
        if(send_base >= nb_frames) {
            break;
        }
        pool_timer = TIMEOUT;
        for(seqnum = send_base; seqnum < nb_frames && seqnum < send_base + window_size; seqnum++) {
            int old_timer = timestamps[seqnum - send_base];
            int new_timer = (int)time(NULL);
            int last_time = new_timer - old_timer;

            if(old_timer >= 0 && last_time > TIMEOUT) {
                timestamps[seqnum - send_base] = new_timer;
                frame.seqnum = seqnum;
                frame.timestamp = new_timer;
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
            if(frame.seqnum < send_base || frame.seqnum >= send_base + window_size) { //out of order
                continue;
            }
            
            window_size = frame.window; // update the window size 
            if(frame.type == PTYPE_ACK) {
                int shift = frame.seqnum - send_base + 1;
                // rearange the timestamp array
                for(int i = 0; i < window_size; i++) {
                    if(i + shift < window_size) {
                        timestamps[i] = timestamps[i + shift];
                    } else {
                        timestamps[i] = 0;
                    }
                }
                send_base = frame.seqnum + 1;
            } 
            else if(frame.type == PTYPE_NACK) {
                int new_timer = (int)time(NULL);
                timestamps[frame.seqnum - send_base] = new_timer;
                frame.timestamp = new_timer;
                if(trtp_send_data(pfile, udpSocket, buf, &frame) < 0) {
                    return -1;
                }
            }
        } // in case of timeout pool_result == 0, nothing to do
    }
    return 1;
}

int trtp_listen(UdpSocket *udpSocket) {   
    int fd_count = 1; 
    int numbytes;
    char buf[MAX_PAYLOAD];
    struct pollfd *pfds = malloc(sizeof(*pfds));

    pfds[0].fd = udpSocket->sock;
    pfds[0].events = POLLIN;
    
    for(;;) { // Main loop
        if (poll(pfds, fd_count, -1) < 1) {
            perror("pollserver failed");
            return -1;
        }
        if (pfds[0].revents & POLLIN) { // If data available
            if ((numbytes = udp_receive(buf, MAX_PAYLOAD, udpSocket)) < 0) {
                return -1;
            }
            printf("%s", buf);
            if(buf[MAX_PAYLOAD - 1] == '\0') {
                break;
            }
        }
    }
    return 1;
}
