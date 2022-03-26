#include "test_trtp.h"
#include "../src/lib/trtp.h"

#include <assert.h>
#include <string.h>

void test_trtp_send_response() {
    printf("test_trtp_send_response start ...\n");
    int port = 12345;
    char *addr = "::1";
    char buf_sender[HEADER_LENGTH];
    char buf_rcv[HEADER_LENGTH];
    UdpSocket socket_sender;
    UdpSocket socket_rcv;
    TrtpFrame frame_sender, frame_rcv;

    assert(udp_open_client(addr, port, &socket_sender) > 0);
    assert(udp_open_server(addr, port, &socket_rcv) > 0);

    frame_sender.type = PTYPE_ACK;
    frame_sender.timestamp = 45898736;
    frame_sender.seqnum = 7;
    frame_sender.window = 5;

    assert(trtp_send_response(&socket_sender, buf_sender, &frame_sender) > 0);
    assert(udp_receive(buf_rcv, HEADER_LENGTH, &socket_rcv) > 0);

    decodeFrame(buf_rcv, &frame_rcv);

    assert(frame_sender.tr == frame_rcv.tr);
    assert(frame_sender.type == frame_rcv.type);
    assert(frame_sender.window == frame_rcv.window);
    assert(frame_sender.seqnum == frame_rcv.seqnum);
    assert(frame_sender.length == frame_rcv.length);
    assert(frame_sender.timestamp == frame_rcv.timestamp);
    assert(frame_sender.crc1 == frame_rcv.crc1);

    udp_close(&socket_sender);
    udp_close(&socket_rcv);

    printf("test_trtp_send_response end ...\n");
}

void test_trtp_send_data() {
    printf("test_trtp_send_data start ...\n");
    int port = 12345;
    char *addr = "::1";
    char *tosend = "tosend.txt";
    char *content = "this is an integration test for the sender and receiver.\n";

    char buf_sender[MAX_FRAME];
    char buf_rcv[MAX_FRAME];
    UdpSocket socket_sender;
    UdpSocket socket_rcv;
    TrtpFrame frame_sender, frame_rcv;

    // creation of the file to send
    FILE *pfile_test = fopen(tosend, "w");
    assert(pfile_test != NULL);
    fputs(content, pfile_test);
    assert(fclose(pfile_test) == 0);

    pfile_test = fopen(tosend, "rb");
    assert(pfile_test != NULL);

    assert(udp_open_client(addr, port, &socket_sender) > 0);
    assert(udp_open_server(addr, port, &socket_rcv) > 0);

    frame_sender.timestamp = 45898736;
    frame_sender.seqnum = 0; //first seqnum
    frame_sender.window = 5;

    assert(trtp_send_data(pfile_test, &socket_sender, buf_sender, &frame_sender) > 0);
    assert(udp_receive(buf_rcv, MAX_FRAME, &socket_rcv) > 0);

    decodeFrame(buf_rcv, &frame_rcv);

    assert(frame_sender.tr == frame_rcv.tr);
    assert(frame_sender.type == frame_rcv.type);
    assert(frame_sender.window == frame_rcv.window);
    assert(frame_sender.seqnum == frame_rcv.seqnum);
    assert(frame_sender.length == frame_rcv.length);
    assert(frame_sender.timestamp == frame_rcv.timestamp);
    assert(frame_sender.crc1 == frame_rcv.crc1);
    assert(frame_sender.crc2 == frame_rcv.crc2);
    assert(strlen(content) == frame_rcv.length);
    
    buf_sender[HEADER_LENGTH + frame_sender.length] = '\0'; //stop the string
    buf_rcv[HEADER_LENGTH + frame_rcv.length] = '\0'; //stop the string

    assert(strcmp(content, buf_sender + HEADER_LENGTH) == 0);
    assert(strcmp(content, buf_rcv + HEADER_LENGTH) == 0);
    assert(fclose(pfile_test) == 0);
    assert(remove(tosend) == 0);

    udp_close(&socket_sender);
    udp_close(&socket_rcv);

    printf("test_trtp_send_data end ...\n");
}