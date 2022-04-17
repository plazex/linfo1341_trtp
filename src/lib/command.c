#include "command.h"
#include "logger.h"

#include <stdlib.h>
#include <string.h>

int cmd_fec = 0;
int cmd_loss = 0;
int cmd_truncated = 0;

int check_file_input(FILE *input) {
    return (fseek(stdin, 0, SEEK_END), ftell(stdin));
}

int init_sender(int argc, char *argv[], char **addr, int *port, FILE **input) {
    char *help = "Help:\n"
        "-f <file_name> : file to send. also can use the '<' to redirect the file content instead\n"
        "-s <stat_file_name> : file to save stats (optional)\n"
        "-l <log_file_name> : file to save logs (optional)\n"
        "-c : enable fec packets (optional)\n"
        "-o <percentage> : percentage of loss packets\n"
        "-t <percentage> : percentage of truncated packets\n"
        "<receiver_address> <receiver_port> : receiver address and port (required in this order)\n\n"
        "Example: sender -o 20 -t 10 -c -f myfile.txt ::1 5623\n";
 
    if(argc < 2) {
        fprintf(stderr, "number of arguments incorrect\n%s", help);
        return -1;
    }

    int argFile = 0, argAddr = 0, argStat = 0, argLog = 0, argLoss = 0, 
        argTrunc = 0;

    cmd_fec = 0;
    cmd_loss = 0;
    cmd_truncated = 0;

    for(int i = 1; i < argc;) {
        if(strcmp(argv[i], "-f") == 0) {
            argFile = i + 1;
            i += 2;
        } else if(strcmp(argv[i], "-s") == 0) {
            argStat = i + 1;
            i += 2;
        } else if(strcmp(argv[i], "-l") == 0) {
            argLog = i + 1;
            i += 2;
        } else if(strcmp(argv[i], "-c") == 0) {
            cmd_fec = 1;
            i += 1;
        } else if(strcmp(argv[i], "-o") == 0) {
            argLoss = i + 1;
            i += 2;
        } else if(strcmp(argv[i], "-t") == 0) {
            argTrunc = i + 1;
            i += 2;
        } else {
            argAddr = i;
            i += 2;
        }
    }

    if(argFile > 0 && argFile < argc) {
        if(!(*input = fopen(argv[argFile], "rb"))) {
            fprintf(stderr, "cannot read the file '%s'\n%s", argv[argFile], help);
            return -1;
        }
    } else {
        if (check_file_input(stdin) > 0) {
            *input = stdin;
        } else {
            fprintf(stderr, "File input is empty\n%s", help);
            return -1;
        }
    }

    if(argAddr > 0 && argAddr + 1 < argc) {
        *addr = argv[argAddr];
        *port = atoi(argv[argAddr + 1]);
        if(strcmp(argv[argAddr], "localhost") == 0) { //inet_pton does not handle localhost
            *addr = "::1";
        }
    } else {
        fprintf(stderr, "incorrect arguments\n%s", help);
        return -1;
    }

    if(argLog > 0 && argLog < argc) {
        initLog(argv[argLog]);
    }
    if(argStat > 0 && argStat < argc) {
        initStat(argv[argStat]);
    }
    if(argLoss > 0 && argLoss < argc) {
        cmd_loss = atoi(argv[argLoss]);
    }
    if(argTrunc > 0 && argTrunc < argc) {
        cmd_truncated = atoi(argv[argTrunc]);
    }
    
    return 1;
}

int init_receiver(int argc, char *argv[], char **addr, int *port) {
    char *help = "Help:\n"
        "-f <file_name> : file to save the received file (optional)\n"
        "-s <stat_file_name> : file to save stats (optional)\n"
        "-l <log_file_name> : file to save logs (optional)\n"
        "-o <percentage> : percentage of loss packets\n"
        "-t <percentage> : percentage of truncated packets\n"
        "<address> <port> : listened address and port (required in this order)\n\n"
        "Example: receiver -o 20 -t 10 -f myfile.txt ::1 5623\n";
    
    if(argc < 2) {
        fprintf(stderr, "number of arguments incorrect\n%s", help);
        return -1;
    }

    int argFile = 0, argAddr = 0, argStat = 0, argLog = 0, argLoss = 0,
        argTrunc = 0;
    
    cmd_loss = 0;
    cmd_truncated = 0;

    for(int i = 1; i < argc; i+=2) {
        if(strcmp(argv[i], "-f") == 0) {
            argFile = i + 1;
        } else if(strcmp(argv[i], "-s") == 0) {
            argStat = i + 1;
        } else if(strcmp(argv[i], "-l") == 0) {
            argLog = i + 1;
        } else if(strcmp(argv[i], "-o") == 0) {
            argLoss = i + 1;
            i += 2;
        } else if(strcmp(argv[i], "-t") == 0) {
            argTrunc = i + 1;
            i += 2;
        } else {
            argAddr = i;
        }
    }
    
    if(argAddr > 0 && argAddr + 1 < argc) {
        *addr = argv[argAddr];
        *port = atoi(argv[argAddr + 1]);
        if(strcmp(argv[argAddr], "localhost") == 0) { //inet_pton does not handle localhost
            *addr = "::1";
        }
    } else {
        fprintf(stderr, "incorrect arguments\n%s", help);
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
    if(argLoss > 0 && argLoss < argc) {
        cmd_loss = atoi(argv[argLoss]);
    }
    if(argTrunc > 0 && argTrunc < argc) {
        cmd_truncated = atoi(argv[argTrunc]);
    }

    return 1;
}