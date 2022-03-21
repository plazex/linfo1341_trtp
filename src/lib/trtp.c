#include "trtp.h"

#define WINDOW_SIZE 4
#define MAX_PAYLOAD 512
#define MAX_FRAME 12 + MAX_PAYLOAD + 4

int trtp_send(FILE *pfile, UdpSocket *udpSocket) {
    int length;
    char buf[MAX_PAYLOAD];
    
    fseek(pfile, 0L, SEEK_END);
    size_t size = ftell(pfile);
    fseek(pfile, 0L, SEEK_SET);

    while ((length = fread(buf, 1, MAX_PAYLOAD, pfile)) > 0) {
        udp_send(buf, length, udpSocket);
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
