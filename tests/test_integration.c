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
    int test_port = 12340;
    char *str_port = "12340";
    char *str_addr = "::1";

    int argc = 5;
    int port;
    char *addr;
    FILE *file;
    UdpSocket socket;
    char *argv[] = {"sender", "-f", integr_test_tosend, str_addr, str_port};

    // creation of the file to send
    FILE *pfile_test = fopen(integr_test_tosend, "w");
    assert(pfile_test != NULL);
    fputs(integr_test_content, pfile_test);
    assert(fclose(pfile_test) == 0);

    assert(init_sender(argc, argv, &addr, &port, &file) > 0);
    assert(strcmp(addr, str_addr) == 0);
    assert(port == test_port);

    assert(udp_open_client(addr, port, &socket) > 0);
    assert(trtp_send(file, &socket) > 0);
}

void* run_receiver() {
    int test_port = 12340;
    char *str_port = "12340";
    char *str_addr = "::1";

    int argc = 5;
    int port;
    char *addr;
    UdpSocket socket;
    char *argv[] = {"receiver", "-f", integr_test_torcv, str_addr, str_port};

    assert(init_receiver(argc, argv, &addr, &port) > 0);
    assert(strcmp(addr, str_addr) == 0);
    assert(port == test_port);

    assert(udp_open_server(addr, port, &socket) > 0);
    assert(trtp_listen(&socket) > 0);
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