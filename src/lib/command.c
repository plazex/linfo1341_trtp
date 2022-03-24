#include "command.h"
#include "logger.h"

#include <stdlib.h>
#include <string.h>

int check_file_input(FILE *input) {
    return (fseek(stdin, 0, SEEK_END), ftell(stdin));
}

int init_sender(int argc, char *argv[], char **addr, int *port, FILE **input) {
    if(argc < 2) {
        fprintf(stderr, "number of arguments incorrect\n");
        return -1;
    }

    int argFile = 0, argAddr = 0, argStat = 0, argLog = 0;

    for(int i = 1; i < argc; i+=2) {
        if(strcmp(argv[i], "-f") == 0) {
            argFile = i + 1;
        } else if(strcmp(argv[i], "-s") == 0) {
            argStat = i + 1;
        } else if(strcmp(argv[i], "-l") == 0) {
            argLog = i + 1;
        } else {
            argAddr = i;
        }
    }

    if(argFile > 0 && argFile < argc) {
        if(!(*input = fopen(argv[argFile], "rb"))) {
            fprintf(stderr, "cannot read the file '%s'\n", argv[argFile]);
            return -1;
        }
    } else {
        if (check_file_input(stdin) > 0) {
            *input = stdin;
        } else {
            fprintf(stderr, "File input is empty\n");
            return -1;
        }
    }

    if(argAddr > 0 && argAddr + 1 < argc) {
        *addr = argv[argAddr];
        *port = atoi(argv[argAddr + 1]);
    } else {
        fprintf(stderr, "incorrect arguments\n");
        return -1;
    }

    if(argLog > 0 && argLog < argc) {
        initLog(argv[argLog]);
    }
    if(argStat > 0 && argStat < argc) {
        initStat(argv[argStat]);
    }
    
    return 1;
}

int init_receiver(int argc, char *argv[], char **addr, int *port) {
    if(argc < 2) {
        fprintf(stderr, "number of arguments incorrect\n");
        return -1;
    }

    int argFile = 0, argAddr = 0, argStat = 0, argLog = 0;

    for(int i = 1; i < argc; i+=2) {
        if(strcmp(argv[i], "-f") == 0) {
            argFile = i + 1;
        } else if(strcmp(argv[i], "-s") == 0) {
            argStat = i + 1;
        } else if(strcmp(argv[i], "-l") == 0) {
            argLog = i + 1;
        } else {
            argAddr = i;
        }
    }
    
    if(argAddr > 0 && argAddr + 1 < argc) {
        *addr = argv[argAddr];
        *port = atoi(argv[argAddr + 1]);
    } else {
        fprintf(stderr, "incorrect arguments\n");
        return -1;
    }

    if(argLog > 0 && argLog < argc) {
        initLog(argv[argLog]);
    } // else write stat to stderr
    if(argStat > 0 && argStat < argc) {
        initStat(argv[argStat]);
    } // else write stat to stderr

    if(argFile > 0 && argFile < argc) {
        initStdout(argv[argFile]);
    } // else the read file will be shown on stdout

    return 1;
}