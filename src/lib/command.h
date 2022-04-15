/*!
 * Implement functions to retrieve command arguments.
 */

#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>

/*! Fec packet enabled */
extern int cmd_fec;
/*! Percentage of loss packets */
extern int cmd_loss;
/*! Percentage of truncated packets */
extern int cmd_truncated;

/*!
 * Check if the given file is not empty.
 * \param input : pointer to the file
 * \return -1 when the file is empty, otherwise an integer > 0.
 */
int check_file_input(FILE *input);

/*!
 * Initialize the needed attributes to send data from the command line arguments.
 * The order is not important except that the port must follow the address.
 * We add two optional arguments for the percentage of loss (-o) and truncated (-t) packets.
 * Help:
 * -f <file_name> : file to send. also can use the '<' to redirect the file content instead
 * -s <stat_file_name> : file to save stats (optional)
 * -l <log_file_name> : file to save logs (optional)
 * -c : enable fec packets (optional)
 * -o <percentage> : percentage of loss packets
 * -t <percentage> : percentage of truncated packets
 * <receiver_address> <receiver_port> : receiver address and port (required in this order)
 * 
 * Example: sender -o 20 -t 10 -c -f myfile.txt ::1 5623
 * This example sends the file myfile.txt to the address ::1 on the port 5623 with the fec
 * packets enabled. Also 20% of loss and 10% of truncated packet.
 * 
 * \param argc : nb of arguments
 * \param argv : arguments
 * \param addr : address IPv6
 * \param port : port number
 * \param fec : use fec data type
 * \param input : pointer to the file to send
 * \return -1 when an error occured, otherwise 1.
 */
int init_sender(int argc, char *argv[], char **addr, int *port, FILE **input);

/*!
 * Initialize the needed attributes to receive data from the command line arguments.
 * The order is not important except that the port must follow the address.
 * We add two optional arguments for the percentage of loss (-o) and truncated (-t) packets.
 * Help:
 * -f <file_name> : file to save the received file (optional)
 * -s <stat_file_name> : file to save stats (optional)
 * -l <log_file_name> : file to save logs (optional)
 * -o <percentage> : percentage of loss packets
 * -t <percentage> : percentage of truncated packets
 * <address> <port> : listened address and port (required in this order)
 * 
 * Example: receiver -o 20 -t 10 -f myfile.txt ::1 5623
 * This example receives the sent file and save it as myfile.txt by listening the address ::1 
 * and the port 5623. Also it enables 20% of loss and 10% of truncated packet (Ack and Nack).
 * \param argc : nb of arguments
 * \param argv : arguments
 * \param addr : address IPv6
 * \param port : port number
 * \return -1 when an error occured, otherwise 1.
 */
int init_receiver(int argc, char *argv[], char **addr, int *port);

#endif //COMMAND_H
