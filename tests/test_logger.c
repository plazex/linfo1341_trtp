#include "test_logger.h"
#include "../src/lib/logger.h"

#include <assert.h>
#include <string.h>

void test_initLog() {
    printf("test_initLog start ...\n");
    const char* test_file = "log_test.txt";
    initLog(test_file);
    assert(log_file != NULL);
    closeFiles();
    assert(log_file == NULL);
    assert(remove(test_file) == 0);
    printf("test_initLog success end ...\n");
}

void test_initStat() {
    printf("test_initStat start ...\n");
    const char* test_file = "stat_test.txt";
    initStat(test_file);
    assert(stat_file != NULL);
    closeFiles();
    assert(stat_file == NULL);
    assert(remove(test_file) == 0);
    printf("test_initStat success end ...\n");
}

void test_initStdout() {
    printf("test_initStdout start ...\n");
    const char* test_file = "output_test.txt";
    initStdout(test_file);
    assert(stdout_file != NULL);
    printf("test_initStdout");
    closeFiles();
    assert(stdout_file == NULL);
    assert(remove(test_file) == 0);
    freopen("/dev/tty", "a", stdout); //redirect to normal only for POSIX
    fprintf(stderr, "test_initStdout success end ...\n");
}

void test_printStat() {
    printf("test_printStat start ...\n");
    const char* test_file = "stat_test.txt";
    initStat(test_file);
    assert(stat_file != NULL);
    printStat(DATA_RECEIVED, 12);
    closeFiles();
    assert(stat_file == NULL);

    int readChar;
    int count = 0;
    char expected[20] = "data_received:12\n";
    stat_file = fopen(test_file, "r");
    while ((readChar = getc(stat_file)) != EOF) {
        assert(readChar == expected[count++]);
    }

    assert(remove(test_file) == 0);
    printf("test_printStat success end ...\n");
}