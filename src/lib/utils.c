#include "utils.h"

const uint8_t PTYPE_FEC  = 0b00;
const uint8_t PTYPE_DATA = 0b01;
const uint8_t PTYPE_ACK  = 0b10;
const uint8_t PTYPE_NACK = 0b11;

void encodeFrame(uint8_t *buf, int size, TrtpFrame* frame) {
    if(size >= TRTP_HEADER_LENGTH) {
        write2First(buf, frame->type);
        writeThird(buf, frame->tr);
        write5Last(buf, frame->window);
        writeUInt16(buf, 1, frame->length);
        buf[3] = frame->seqnum;
        writeUInt32(buf, 4, frame->timestamp);
        writeUInt32(buf, 8, frame->crc1);
        memcpy(buf + TRTP_HEADER_LENGTH, frame->payload, frame->length);

        if(size >= TRTP_HEADER_LENGTH + frame->length + 4) {
            writeUInt32(buf, TRTP_HEADER_LENGTH + frame->length, frame->crc2);
        }
    }
}

void decodeFrame(char *buf, int size, TrtpFrame* frame) {
    if(size >= TRTP_HEADER_LENGTH) {
        frame->type = read2First(buf[0]);
        frame->tr = readThird(buf[0]);
        frame->window = read5Last(buf[0]);
        frame->length = readUInt16(buf, 1);
        frame->seqnum = (uint8_t)buf[3];
        frame->timestamp = readUInt32(buf, 4);
        frame->crc1 = readUInt32(buf, 8);
        frame->payload = &buf[TRTP_HEADER_LENGTH];

        if(size >= TRTP_HEADER_LENGTH + frame->length + 4) {
            frame->crc2 = readUInt32(buf, TRTP_HEADER_LENGTH + frame->length);
        }
    }
}

void write2First(uint8_t *buf, uint8_t value) {
    buf[0] |= (uint8_t)(0b11000000 & (value << 6));
}
void writeThird(uint8_t *buf, uint8_t value) {
    buf[0] |= (uint8_t)(0b00100000 & (value << 5));
}
void write5Last(uint8_t *buf, uint8_t value) {
    buf[0] |= (uint8_t)(0b00011111 & value);
}
void writeUInt16(uint8_t *buf, int index, uint16_t value) {
    buf[index++] = (uint8_t)(value & 0xff);
	buf[index] = (uint8_t)((value & 0xff00) >> 8);
}
void writeUInt32(uint8_t *buf, int index, uint32_t value) {
    buf[index++] = (uint8_t)(value & 0xff);
	buf[index++] = (uint8_t)((value & 0xff00) >> 8);
	buf[index++] = (uint8_t)((value & 0xff0000) >> 16);
	buf[index] = (uint8_t)((value & 0xff000000) >> 24);
}

uint8_t read2First(char value) {
    return (uint8_t)((value & 0b11000000) >> 6);
}
uint8_t readThird(char value) {
    return (uint8_t)((value & 0b00100000) >> 5);
}
uint8_t read5Last(char value) {
    return (uint8_t)(value & 0b00011111);
}
uint16_t readUInt16(char *buf, int index) {
    return (uint16_t)((uint8_t)(buf[1] << 0) | (uint8_t)(buf[2] << 8));
}
uint32_t readUInt32(char *buf, int index) {
    return (uint32_t)((uint8_t)(buf[index] << 0) | ((uint8_t)buf[index + 1] << 8) |
		((uint8_t)buf[index + 2] << 16) | ((uint8_t)buf[index + 3] << 24));
}