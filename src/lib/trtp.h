/*!
 * Implement trtp data structure and functions.
 */

#ifndef TRTP_H
#define TRTP_H

#include "udp.h"
#include "utils.h"

#include <poll.h>


/*!
 * Send the given file using trtp. 
 * \param pfile : file to send
 * \param udpSocket : the socket and address info to listen
 * \return -1 when an error occured, otherwise 1
 */
int trtp_send(FILE *pfile, UdpSocket *udpSocket);

/*!
 * Listen continuously to the given socket. 
 * \param udpSocket : the socket and address info to listen
 * \return -1 when an error occured, otherwise 1
 */
int trtp_listen(UdpSocket *udpSocket);


#endif //TRTP_H