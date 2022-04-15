#include "../lib/trtp.h"
#include "../lib/udp.h"
#include "../lib/logger.h"
#include "../lib/command.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    int port;
    char *addr;
    FILE *pfile;
    UdpSocket udp_sock;

    if(init_sender(argc, argv, &addr, &port, &pfile) < 0) {
        return EXIT_FAILURE;
    }

    if(udp_open_client(addr, port, &udp_sock) < 0) {
        return EXIT_FAILURE;
    }
    
    if(trtp_send(pfile, &udp_sock) < 0) {
        return EXIT_FAILURE;
    }
    closeFiles();
    fclose(pfile);

    return 0;
}