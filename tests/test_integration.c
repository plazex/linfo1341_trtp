#include "test_integration.h"
#include "../src/lib/trtp.h"
#include "../src/lib/udp.h"
#include "../src/lib/logger.h"
#include "../src/lib/command.h"

#include <assert.h>
#include <string.h>
#include <pthread.h>

char *integr_test_tosend = "tosend.txt";
char *integr_test_torcv = "torcv.txt";
char *integr_test_content = 
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n"
    "this is an integration test for the sender and receiver, where each process is running in its own thread.\n";


void* run_sender() {
    int testPort_sender = 12340;
    int testPort_rcv = 12341;
    char *strPort_sender = "12340";
    char *strAddr = "::1";

    int argc_sender = 5;
    int port_sender;
    int port_rcv = testPort_rcv;
    char *addr_rcv;
    char *addr_sender;
    FILE *file_sender;
    UdpSocket socket_sender;
    char *argv_sender[] = {"sender", "-f", integr_test_tosend, strAddr, strPort_sender};

    // creation of the file to send
    FILE *pfile_test = fopen(integr_test_tosend, "w");
    assert(pfile_test != NULL);
    fputs(integr_test_content, pfile_test);
    assert(fclose(pfile_test) == 0);

    assert(init_sender(argc_sender, argv_sender, &addr_sender, &port_sender, &file_sender) > 0);
    assert(strcmp(addr_sender, strAddr) == 0);
    assert(port_sender == testPort_sender);

    assert(udp_open(addr_sender, port_rcv, port_sender, &socket_sender) > 0);
    assert(trtp_send(file_sender, &socket_sender) > 0);
}

void* run_receiver() {
    int testPort_sender = 12340;
    int testPort_rcv = 12341;
    char *strPort_sender = "12340";
    char *strPort_rcv = "12341";
    char *strAddr = "::1";

    int argc_sender = 5;
    int argc_rcv = 5;
    int port_sender = testPort_sender;
    int port_rcv;
    //char *addr_sender = strAddr;
    char *addr_rcv;
    FILE *file_sender, *file_rcv;
    UdpSocket socket_sender;
    UdpSocket socket_rcv;
    char *argv_rcv[] = {"receiver", "-f", integr_test_torcv, strAddr, strPort_rcv};

    assert(init_receiver(argc_rcv, argv_rcv, &addr_rcv, &port_rcv) > 0);
    assert(strcmp(addr_rcv, strAddr) == 0);
    assert(port_rcv == testPort_rcv);

    assert(udp_open(addr_rcv, port_sender, port_rcv, &socket_rcv) > 0);
    assert(trtp_listen(&socket_rcv) > 0);
}

void test_send_receive_file() {
    printf("test_send_receive_file start ...\n");
    pthread_t pt_sender, pt_rcv;

    pthread_create(&pt_rcv, NULL, &run_receiver, NULL);
    pthread_create(&pt_sender, NULL, &run_sender, NULL);

    pthread_join(pt_rcv, NULL);
    pthread_join(pt_sender, NULL);
    closeFiles();

    int readChar;
    int count = 0;
    FILE *file_rcv = fopen(integr_test_torcv, "r");

    assert(file_rcv != NULL);

    while ((readChar = getc(file_rcv)) != EOF) {
        assert(readChar == integr_test_content[count++]);
    }
    assert(count == strlen(integr_test_content));

    assert(fclose(file_rcv) == 0);
    assert(remove(integr_test_tosend) == 0);
    assert(remove(integr_test_torcv) == 0);

    freopen("/dev/tty", "a", stdout); //redirect to normal only for POSIX
    
    printf("test_send_receive_file success end ...\n");
}