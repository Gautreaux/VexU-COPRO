#ifndef __VEX_SERIAL__
#define __VEX_SERIAL__

#include <string.h>
#include <stdint.h>
#include <cstdio>
#include <unistd.h>

#define MAX_MESSAGE_LEN 100
#define STREAM_SZ_REQUIRED (MAX_MESSAGE_LEN + 2)
#define MAX_MESSAGES_IN_FLIGHT 10
#define ILLEGAL_CHAR 'p'

namespace VexSerial{
extern int SerialFD;

// escape the p characters from the message
//  size should not include trailing null character
//  but trailing null should probably be present for safety
void serializeMsg(const uint8_t* const msg, uint8_t* dst, const uint8_t size);

// unescape the p characters from the message
//  size should be number of bytes read from the stream (which includes the null)
void deserializeMsg(const uint8_t* const ser_msg, uint8_t* dst, const uint8_t size);

// send a preserialized message directly
//  size should be number of bytes to send
inline void sendMessageDirectly(const uint8_t* const msg, const uint8_t size){
    printf("SerialFD = %d\n", SerialFD);
    write(SerialFD, msg, size);
}

// serialize and send a message
//  size is # data byes in msg to send, not counting any null terminator
//      null is not required but recommended
inline void sendMessage(const uint8_t* const msg, const uint8_t size){
    uint8_t serialBuffer[STREAM_SZ_REQUIRED];
    memset(serialBuffer, 0, STREAM_SZ_REQUIRED);
    serializeMsg(msg, serialBuffer, size);
    sendMessageDirectly(serialBuffer, size + 2);
}

// receive and deserialize directly into a buffer
void receiveMessage(uint8_t* const dst, uint8_t& size);
}

#endif //__VEX_SERIAL__