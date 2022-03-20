#include "test_integration.h"
#include "../src/lib/trtp.h"
#include "../src/lib/udp.h"
#include "../src/lib/logger.h"
#include "../src/lib/command.h"

#include <assert.h>
#include <string.h>

void test_send_receive_file() {
    printf("test_send_receive_file start ...\n");
    int testPort = 12345;
    char *strAddr = "::1";
    char *strPort = "12345";
    char *tosend = "tosend.txt";
    char *torcv = "torcv.txt";
    char *content = "this is an integration test for the sender and receiver.\n";

    int argc_sender = 5;
    int argc_rcv = 5;
    int port_sender;
    int port_rcv;
    char *addr_sender;
    char *addr_rcv;
    FILE *file_sender, *file_rcv;
    UdpSocket socket_sender;
    UdpSocket socket_rcv;
    char *argv_sender[] = {"sender", "-f", tosend, strAddr, strPort};
    char *argv_rcv[] = {"receiver", "-f", torcv, strAddr, strPort};

    // creation of the file to send
    FILE *pfile_test = fopen(tosend, "w");
    assert(pfile_test != NULL);
    fputs(content, pfile_test);
    assert(fclose(pfile_test) == 0);

    assert(init_sender(argc_sender, argv_sender, &addr_sender, &port_sender, &file_sender) > 0);
    assert(strcmp(addr_sender, strAddr) == 0);
    assert(port_sender == testPort);

    assert(init_receiver(argc_rcv, argv_rcv, &addr_rcv, &port_rcv) > 0);
    assert(strcmp(addr_rcv, strAddr) == 0);
    assert(port_rcv == testPort);

    assert(udp_open_client(addr_sender, port_sender, &socket_sender) > 0);
    assert(udp_open_server(addr_rcv, port_rcv, &socket_rcv) > 0);

    assert(trtp_send(file_sender, &socket_sender) > 0);
    assert(trtp_listen(&socket_rcv) > 0);
    closeFiles();

    int readChar;
    int count = 0;
    file_rcv = fopen(torcv, "r");

    assert(file_rcv != NULL);

    while ((readChar = getc(file_rcv)) != EOF) {
        assert(readChar == content[count++]);
    }
    assert(count > 0);

    assert(fclose(file_sender) == 0);
    assert(fclose(file_rcv) == 0);
    assert(remove(tosend) == 0);
    assert(remove(torcv) == 0);

    freopen("/dev/tty", "a", stdout); //redirect to normal only for POSIX
    
    printf("test_send_receive_file success end ...\n");
}