#include "logger.h"

FILE* log_file = NULL;
FILE* stat_file = NULL;
FILE* stdout_file = NULL;
const char* stats[15] = { 
    "data_sent", 
    "data_received", 
    "data_truncated_received",
    "fec_sent",
    "fec_received",
    "ack_sent",
    "ack_received",
    "nack_sent",
    "nack_received",
    "packet_ignored",
    "min_rtt",
    "max_rtt",
    "packet_retransmitted",
    "packet_duplicated",
    "packet_recovered"
};

void initLog(const char* filename) {
    // delete file if it exists
    remove(filename);
    log_file = freopen(filename,"a", stderr);
}

void initStat(const char* filename) {
    // delete file if it exists
    remove(filename);
    stat_file = fopen(filename,"a");
}

void initStdout(const char* filename) {
    // delete file if it exists
    remove(filename);
    stdout_file = freopen(filename,"ab", stdout);
}

void closeFiles() {
    if(log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
    if(stat_file != NULL) {
        fclose(stat_file);
        stat_file = NULL;
    }
    if(stdout_file != NULL) {
        fclose(stdout_file);
        stdout_file = NULL;
    }
}

void printStat(int type, int value) {
    if(stat_file != NULL) {
        fprintf(stat_file, "%s:%d\n", stats[type], value);
    } else {
        fprintf(stderr, "%s:%d\n", stats[type], value);
    }
}

void printFile(const char* filename) {
    FILE * pFile;
    int readChar;
    pFile = fopen (filename,"w");
    if (pFile != NULL)
    {
        while ((readChar = getc(pFile)) != EOF) {
            putchar(readChar);
        }
        fclose (pFile);
    } else {
        fprintf(stderr, "cannot read the file '%s'\n", filename);
    }
}