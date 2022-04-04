#include "udp.h"

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int udp_open_server(const char *addr, int port, UdpSocket *udpSocket) {
    if((udpSocket->sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    //setting server config
    udpSocket->node.sin6_family = AF_INET6;
    udpSocket->node.sin6_port = htons(port);
    if(inet_pton(AF_INET6, addr, &udpSocket->node.sin6_addr) == 0) {
        perror("wrong address passed");
        close(udpSocket->sock);
        return -1;
    }
       
    // Bind the socket with the server address
    if(bind(udpSocket->sock, (struct sockaddr *)&udpSocket->node, 
        sizeof(udpSocket->node)) < 0) {
        
        perror("socket binding failed");
        close(udpSocket->sock);
        return -1;
    }
    return 1;
}

int udp_open_client(const char *addr, int port, UdpSocket *udpSocket) {
    if((udpSocket->sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    //setting client config
	udpSocket->node.sin6_family = AF_INET6;
	udpSocket->node.sin6_port = htons(port);
    if(inet_pton(AF_INET6, addr, &udpSocket->node.sin6_addr) == 0) {
        perror("wrong address passed");
        close(udpSocket->sock);
        return -1;
    }
    return 1;
}

void udp_close(UdpSocket *udpSocket) {
    if(udpSocket != NULL && udpSocket->sock >= 0) {
        close(udpSocket->sock);
        udpSocket->sock = -1;
    }
}

int udp_send(const char *buf, int size, UdpSocket *udpSocket) {
    int numbytes;
	if((numbytes = sendto(udpSocket->sock, buf, size, 0, 
        (struct sockaddr *)&udpSocket->node, sizeof(udpSocket->node))) < 0) {
        
        perror("udp sending failed");
        return -1;
    }
    
#ifdef TEST_DEBUG
    char s[INET6_ADDRSTRLEN];
    fprintf(stderr, "client: sent %d bytes to [%s, %d]\n", numbytes, 
        inet_ntop(udpSocket->node.sin6_family, &udpSocket->node.sin6_addr, s, INET6_ADDRSTRLEN),
        (int)ntohs(udpSocket->node.sin6_port));
#endif 
    
    return numbytes;
}

int udp_receive(char *buf, int size, UdpSocket *udpSocket) {
    int numbytes;
    socklen_t addr_len;

    int addrLen = sizeof(udpSocket->node);
	if ((numbytes = recvfrom(udpSocket->sock, buf, size, 0, 
        (struct sockaddr *)&udpSocket->node, &addrLen)) < 0) {
        
		perror("udp receiving failed");
		return -1;
	}
#ifdef TEST_DEBUG
    char s[INET6_ADDRSTRLEN];
    fprintf(stderr, "server: got packet %d bytes long from [%s, %d]\n", numbytes,
        inet_ntop(udpSocket->node.sin6_family, &udpSocket->node.sin6_addr, s, INET6_ADDRSTRLEN),
        (int)ntohs(udpSocket->node.sin6_port));
#endif 

    return numbytes;
}