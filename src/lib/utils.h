/*!
 *  Keeps global constants and enums.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <string.h>

#define TRTP_HEADER_LENGTH 12

/*!
 *  Stat format enum.
 */
enum StatType {
    data_sent = 0, 
    data_received = 1, 
    data_truncated_received = 2,
    fec_sent = 3,
    fec_received = 4,
    ack_sent = 5,
    ack_received = 6,
    nack_sent = 7,
    nack_received = 8,
    packet_ignored = 9,
    min_rtt = 10,
    max_rtt = 11,
    packet_retransmitted = 12,
    packet_duplicated = 13,
    packet_recovered = 14
};

/*!
 *  Trtp frame data structure.
 */
typedef struct TrtpFrame {
	uint8_t type;		// data type (1 byte)
	uint8_t tr;		    // truncated (1 byte)
	uint8_t window;		// rcv window size (1 byte)
	uint16_t length;	// data size (2 bytes)
	uint8_t seqnum;	    // sequence num (1 byte)
    uint32_t timestamp;	// timestamp (4 bytes)
    uint32_t crc1;	    // crc1 encoding (4 bytes)
    uint32_t crc2;	    // crc2 encoding (4 bytes)
    uint8_t *payload;	// data (sequence of bytes)
} TrtpFrame;

/*!
 *  Frame type constants.
 */
const uint8_t PTYPE_FEC;
const uint8_t PTYPE_DATA;
const uint8_t PTYPE_ACK;
const uint8_t PTYPE_NACK;

/*!
 * Encode the given frame in a buffer.
 * It requires that the buffer is initialized and empty.
 * \param buf : data buffer
 * \param size : buffer size
 * \param frame : the frame to encode
 */
void encodeFrame(uint8_t *buf, int size, TrtpFrame* frame);

/*!
 * Decode the given buffer and fill the frame
 * \param buf : data buffer
 * \param size : buffer size
 * \param frame : the frame to fill
 */
void decodeFrame(char *buf, int size, TrtpFrame* frame);

void write2First(uint8_t *buf, uint8_t value);
void writeThird(uint8_t *buf, uint8_t value);
void write5Last(uint8_t *buf, uint8_t value);
void writeUInt16(uint8_t *buf, int index, uint16_t value);
void writeUInt32(uint8_t *buf, int index, uint32_t value);

uint8_t read2First(char value);
uint8_t readThird(char value);
uint8_t read5Last(char value);
uint16_t readUInt16(char *buf, int index);
uint32_t readUInt32(char *buf, int index);

#endif //UTILS_H