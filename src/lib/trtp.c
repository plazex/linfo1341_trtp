#include "trtp.h"

#define MAX_PAYLOAD 512

int trtp_send(FILE *pfile, UdpSocket *udpSocket) {
    char buf[MAX_PAYLOAD];
    while (fgets(buf, MAX_PAYLOAD, pfile)) {
        udp_send(buf, MAX_PAYLOAD, udpSocket);
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
            if(buf[MAX_PAYLOAD - 2] == '\0') {
                break;
            }
        }
    }
    return 1;
}
