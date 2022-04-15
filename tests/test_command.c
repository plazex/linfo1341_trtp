#include "test_command.h"
#include "../src/lib/command.h"
#include "../src/lib/logger.h"

#include <assert.h>
#include <string.h>

void test_init_sender() {
    printf("test_init_sender start ...\n");
    int argc = 6;
    int port = 0;
    char *addr = NULL;

    int testPort = 12345;
    int testLoss = 30;
    int testTruncated = 20;
    char *strAddr = "::1";
    char *strPort = "12345";
    char *loss = "30";
    char * truncated = "20";
    
    char *filename = "test.txt";
    char *fakefile = "fake.txt";
    char *argvfake1[] = {""};
    char *argvfake2[] = {"sender", "-f", fakefile, strAddr, strPort, "-c"};
    char *argvfake3[] = {"sender", "no -f", fakefile, strAddr, strPort, "-c"};
    char *argv1[] = {"sender", "-f", filename, strAddr, strPort};
    char *argv2[] = {"sender", strAddr, strPort, "-f", filename, "-c"};
    char *argv3[] = { "sender", "-c", strAddr, strPort, "-o", loss, "-t", truncated};
    FILE *pfiletest = NULL;
    FILE *pfile = fopen(filename, "w");

    // should failed
    assert(init_sender(1, argvfake1, &addr, &port, &pfiletest) < 0);
    assert(init_sender(argc, argvfake2, &addr, &port, &pfiletest) < 0);
    assert(init_sender(argc, argvfake3, &addr, &port, &pfiletest) < 0);

    assert(pfile != NULL);
    assert(init_sender(5, argv1, &addr, &port, &pfiletest) > 0);
    assert(strcmp(addr, strAddr) == 0);
    assert(port == testPort);
    assert(cmd_fec == 0);
    assert(pfiletest != NULL);
    assert(fclose(pfiletest) == 0);

    port = 0; cmd_fec = -1; addr = NULL; pfiletest = NULL;

    assert(init_sender(argc, argv2, &addr, &port, &pfiletest) > 0);
    assert(strcmp(addr, strAddr) == 0);
    assert(port == testPort);
    assert(cmd_fec == 1);
    assert(pfiletest != NULL);
    
    assert(fclose(pfile) == 0);
    assert(remove(filename) == 0);

    argc = 8; port = 0; cmd_fec = -1; addr = NULL;
    pfile = freopen(filename,"w",stdin);
    assert(pfile != NULL);
    fputs("add something!", pfile);
    assert(init_sender(argc, argv3, &addr, &port, &stdin) > 0);
    assert(strcmp(addr, strAddr) == 0);
    assert(port == testPort);
    assert(cmd_fec == 1);
    assert(cmd_loss == testLoss);
    assert(cmd_truncated == testTruncated);

    assert(fclose(pfile) == 0);
    assert(remove(filename) == 0);
    
    printf("test_init_sender success end ...\n");
}

void test_init_receiver() {
    printf("test_init_receiver start ...\n");
    int argc = 5;
    int port = 0;
    char *addr = NULL;

    int testPort = 12345;
    char *strAddr = "::1";
    char *strPort = "12345";
    
    char *filename = "test.txt";
    char *statfile = "stat.txt";
    char *argvfake1[] = {""};
    char *argvfake2[] = {"receiver", "-s", statfile, strPort}; // no address
    char *argv1[] = {"receiver", "-f", filename, strAddr, strPort};
    char *argv2[] = {"receiver", strAddr, strPort, "-f", filename, "-s", statfile};

    // should failed
    assert(init_receiver(1, argvfake1, &addr, &port) < 0);
    assert(init_receiver(4, argvfake2, &addr, &port) < 0);

    assert(init_receiver(argc, argv1, &addr, &port) > 0);
    assert(strcmp(addr, strAddr) == 0);
    assert(port == testPort);
    assert(stdout_file != NULL);
    
    closeFiles();
    argc = 7; port = 0; addr = NULL;

    assert(init_receiver(argc, argv2, &addr, &port) > 0);
    assert(strcmp(addr, strAddr) == 0);
    assert(port == testPort);
    assert(stdout_file != NULL);
    assert(stat_file != NULL);
    
    closeFiles();
    assert(remove(statfile) == 0);
    assert(remove(filename) == 0);
    
    printf("test_init_receiver success end ...\n");
}