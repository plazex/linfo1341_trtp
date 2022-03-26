/*!
 * Implement trtp data structure and functions.
 */

#ifndef TRTP_H
#define TRTP_H

#include "udp.h"
#include "utils.h"

#include <poll.h>

/*!
 * Send a response frame (ACK or NACK) to the given socket. 
 * \param udpSocket: the receiver socket
 * \param buf: buffer to write the frame
 * \param frame: pointer to the frame to send
 * \return -1 when an error occured, otherwise 1
 */
int trtp_send_response(UdpSocket *udpSocket, char *buf, TrtpFrame *frame);

/*!
 * Send a data frame to the given socket. 
 * \param pfile: file to send
 * \param udpSocket: the receiver socket
 * \param buf: buffer to write the frame
 * \param frame: pointer to the frame to send
 * \return -1 when an error occured, otherwise 1
 */
int trtp_send_data(FILE *pfile, UdpSocket *udpSocket, char *buf, TrtpFrame *frame);

/*!
 * Send the given file using trtp. 
 * \param pfile: file to send
 * \param udpSocket: the socket and address info to listen
 * \return -1 when an error occured, otherwise 1
 */
int trtp_send(FILE *pfile, UdpSocket *udpSocket);

/*!
 * Listen continuously to the given socket. 
 * \param udpSocket: the socket and address info to listen
 * \return -1 when an error occured, otherwise 1
 */
int trtp_listen(UdpSocket *udpSocket);


#endif //TRTP_H