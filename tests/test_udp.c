#include "test_udp.h"
#include "../src/lib/udp.h"

#include <assert.h>
#include <string.h>

void test_udp_open() {
    printf("test_udp_open start ...\n");

    UdpSocket udpSocket;
    const char* addr = "::1";
    const char* fakeAddr = "unknown";
    int port = 1239;

    //chek fake address: should failed
    assert(udp_open_server(fakeAddr, port, &udpSocket) < 0);
    assert(udp_open_client(fakeAddr, port, &udpSocket) < 0);
    // check openning server
    assert(udp_open_server(addr, port, &udpSocket) > 0);
    assert(udpSocket.sock >= 0);
    udp_close(&udpSocket);
    assert(udpSocket.sock < 0);
    // check openning client
    assert(udp_open_client(addr, port, &udpSocket) > 0);
    assert(udpSocket.sock >= 0);
    udp_close(&udpSocket);
    assert(udpSocket.sock < 0);

    printf("test_udp_open end ...\n");
}

void test_udp_send_receive() {
    printf("test_udp_send start ...\n");

    UdpSocket socketClient;
    UdpSocket socketServer;
    const char* addr = "::1";
    int port = 1239;
    int len = 10;
    const char* sendBuffer = "test";
    char recvBuffer[len];

    // check client openning
    assert(udp_open_client(addr, port, &socketClient) > 0);
    // check server openning
    assert(udp_open_server(addr, port, &socketServer) > 0);
    // check client sending
    assert(udp_send(sendBuffer, sizeof(sendBuffer), &socketClient) > 0);
    // check server receiving
    assert(udp_receive(recvBuffer, len, &socketServer) > 0);
    // check the sent text with the received one
    assert(strcmp(sendBuffer,recvBuffer) == 0);

    printf("test_udp_send end ...\n");
}