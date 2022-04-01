/*!
 *  Keeps global constants and enums.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <string.h>

#define MAX_WINDOW_SIZE 31
#define HEADER_LENGTH 12
#define MAX_PAYLOAD 512
#define MAX_FRAME HEADER_LENGTH + MAX_PAYLOAD + 4

/*!
 *  Stat format enum.
 */
enum StatType {
    DATA_SENT = 0, 
    DATA_RECEIVED = 1, 
    DATA_TRUNCATED_RECEIVED = 2,
    FEC_SENT = 3,
    FEC_RECEIVED = 4,
    ACK_SENT = 5,
    ACK_RECEIVED = 6,
    NACK_SENT = 7,
    NACK_RECEIVED = 8,
    PACKET_IGNORED = 9,
    MIN_RTT = 10,
    MAX_RTT = 11,
    PACKET_RETRANSMITTED = 12,
    PACKET_DUPLICATED = 13,
    PACKET_RECOVERED = 14
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
 * \param frame : the frame to encode
 */
void encodeFrame(uint8_t *buf, TrtpFrame* frame);

/*!
 * Decode the given buffer and fill the frame
 * \param buf : data buffer
 * \param size : buffer size
 * \param frame : the frame to fill
 */
void decodeFrame(char *buf, TrtpFrame* frame);

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