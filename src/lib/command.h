/*!
 * Implement functions to retrieve command arguments.
 */

#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>

/*!
 * Check if the given file is not empty.
 * \param input : pointer to the file
 * \return -1 when the file is empty, otherwise an integer > 0.
 */
int check_file_input(FILE *input);

/*!
 * Initialize the needed attributes to send data from
 * the command line arguments.
 * \param argc : nb of arguments
 * \param argv : arguments
 * \param addr : address IPv6
 * \param port : port number
 * \param input : pointer to the file to send
 * \return -1 when an error occured, otherwise 1.
 */
int init_sender(int argc, char *argv[], char **addr, int *port, FILE **input);

/*!
 * Initialize the needed attributes to receive data from
 * the command line arguments.
 * \param argc : nb of arguments
 * \param argv : arguments
 * \param addr : address IPv6
 * \param port : port number
 * \return -1 when an error occured, otherwise 1.
 */
int init_receiver(int argc, char *argv[], char **addr, int *port);

#endif //COMMAND_H
