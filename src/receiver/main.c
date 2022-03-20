#include "../lib/trtp.h"
#include "../lib/udp.h"
#include "../lib/logger.h"
#include "../lib/command.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    int port;
    char *addr;
    UdpSocket udp_sock;

    if(init_receiver(argc, argv, &addr, &port) < 0) {
        return EXIT_FAILURE;
    }

    if(udp_open_server(addr, port, &udp_sock) < 0) {
        return EXIT_FAILURE;
    }
    
    if(trtp_listen(&udp_sock) < 0) {
        return EXIT_FAILURE;
    }
    closeFiles();

    return 0;
}