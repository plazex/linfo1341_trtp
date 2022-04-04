/*!
 * Defines udp data structure and functions.
 * Adapted source from: https://beej.us/guide/bgnet/html//index.html
 */

#ifndef UDP_H
#define UDP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

/*!
 *  UDP socket data structure.
 *  Keeps the socket id and the address info.
 */
typedef struct UdpSocket
{
    int sock;
    struct sockaddr_in6 node; // address node
} UdpSocket;

/*!
 * Get sockaddr IPv4 or IPv6
 * \param sa : socket address struct
 * \return an IPv4 or IPv6 address.
 */
void *get_in_addr(struct sockaddr *sa);

/*!
 * Opens the server socket for listening to incomming connection.
 * \param addr : server address
 * \param port : listening port number
 * \param udpSocket : the socket and address info to initialize
 * \return -1 when an error occured, otherwise 1.
 */
int udp_open_server(const char *addr, int port, UdpSocket *udpSocket);

/*!
 * Opens the client socket for sending data.
 * \param addr : server address
 * \param port : server listening port number
 * \param udpSocket : the socket and address info to initialize
 * \return -1 when an error occured, otherwise 1.
 */
int udp_open_client(const char *addr, int port, UdpSocket *udpSocket);

/*!
 * Close the given udp socket.
 * \param udpSocket : the socket and address info to close/free
 */
void udp_close(UdpSocket *udpSocket);

/*!
 * Sends data to the client address.
 * \param buf : data buffer
 * \param size : buffer size
 * \param udpSocket : the socket and address info to use for sending
 * \return -1 when an error occured, otherwise the lenght of sent data
 */
int udp_send(const char *buf, int size, UdpSocket *udpSocket);

/*!
 * Receives data from the client address.
 * \param buf : data buffer
 * \param size : buffer size
 * \param udpSocket : the socket and address info to listen
 * \return -1 when an error occured, otherwise the lenght of received data
 */
int udp_receive(char *buf, int size, UdpSocket *udpSocket);

#endif //UDP_H