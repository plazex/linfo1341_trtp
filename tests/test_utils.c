#include "test_utils.h"
#include "../src/lib/utils.h"

#include <stdio.h>
#include <assert.h>

void test_write2First() {
    printf("test_write2First start ...\n");
    uint8_t test = 0; // empty
    write2First(&test, PTYPE_FEC);
    assert(test == (uint8_t)0b00000000);
    write2First(&test, PTYPE_DATA);
    assert(test == (uint8_t)0b01000000);
    test = 0; // empty the 2 first bits
    write2First(&test, PTYPE_ACK);
    assert(test == (uint8_t)0b10000000);
    test = 0; // empty the 2 first bits
    write2First(&test, PTYPE_NACK);
    assert(test == (uint8_t)0b11000000);
    printf("test_write2First success end ...\n");
}

void test_writeThird() {
    printf("test_writeThird start ...\n");
    uint8_t test = 0; // empty
    writeThird(&test, 0b0);
    assert(test == (uint8_t)0b00000000);
    writeThird(&test, 0b1);
    assert(test == (uint8_t)0b00100000);
    printf("test_writeThird success end ...\n");
}

void test_write5Last() {
    printf("test_write5Last start ...\n");
    uint8_t test = 0; // empty
    write5Last(&test, 0b11111); //31
    assert(test == (uint8_t)0b00011111);
    test = 0; // empty the 5 last bits
    write5Last(&test, 0b10011); //19
    assert(test == (uint8_t)0b00010011);
    printf("test_write5Last success end ...\n");
}

void test_read2First() {
    printf("test_read2First start ...\n");
    char test = 0b00000000; // PTYPE_FEC = 0b00
    assert(read2First(test) == PTYPE_FEC);
    test = 0b01000000; // PTYPE_DATA = 0b01
    assert(read2First(test) == PTYPE_DATA);
    test = 0b10000000; // PTYPE_ACK = 0b10
    assert(read2First(test) == PTYPE_ACK);
    test = 0b11000000; // PTYPE_ACK = 0b11
    assert(read2First(test) == PTYPE_NACK);
    printf("test_read2First success end ...\n");
}

void test_readThird() {
    printf("test_readThird start ...\n");
    char test = 0b00000000; // 0
    assert(readThird(test) == 0);
    test = 0b00100000; // 1
    assert(readThird(test) == 1);
    printf("test_readThird success end ...\n");
}

void test_read5Last() {
    printf("test_read5Last start ...\n");
    char test = 0b10111111; // last 5 bit = 31
    assert(read5Last(test) == 31);
    test = 0b01000101; // last 5 bit = 5
    assert(read5Last(test) == 5);
    printf("test_read5Last success end ...\n");
}

void test_read_write_UInt16() {
    printf("test_read_write_UInt16 start ...\n");
    char buf[2];
    uint16_t test = 356;
    writeUInt16(buf, 0, test);
    assert(readUInt16(buf, 0) == test);
    printf("test_read_write_UInt16 success end ...\n");
}

void test_read_write_UInt32() {
    printf("test_read_write_UInt32 start ...\n");
    char buf[4];
    uint32_t test = 8796545;
    writeUInt32(buf, 0, test);
    assert(readUInt32(buf, 0) == test);
    printf("test_read_write_UInt32 success end ...\n");
}

void test_encode_decodeFrame() {
    printf("test_encode_decodeFrame start ...\n");
    int size = 20;
    uint8_t data = 11;
    uint8_t buf[size];
    TrtpFrame frame1;
    TrtpFrame frame2 = {
        PTYPE_DATA, //type
        0, //tr
        23, //window
        4, //length
        65, //seqnum
        45892, //timestamp
        12879, //crc1
        782543, //crc2
        &data //payload
    };

    memset(buf, 0, size); //clear buffer
    encodeFrame(buf, &frame2);
    decodeFrame(buf, &frame1);
    
    assert(frame1.type == frame2.type);
    assert(frame1.tr == frame2.tr);
    assert(frame1.window == frame2.window);
    assert(frame1.length == frame2.length);
    assert(frame1.seqnum == frame2.seqnum);
    assert(frame1.timestamp == frame2.timestamp);
    assert(frame1.crc1 == frame2.crc1);
    assert(frame1.crc2 == frame2.crc2);

    printf("test_encode_decodeFrame success end ...\n");
}