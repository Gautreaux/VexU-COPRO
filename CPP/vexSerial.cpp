#include "vexSerial.h"

int VexSerial::SerialFD = 0;

void VexSerial::serializeMsg(const uint8_t* const msg, uint8_t* dst, const uint8_t size){

    const uint8_t* nextRead = msg;
    uint8_t* nextWrite = dst;
    uint8_t bytesCopied = 0;

    while(bytesCopied < size){
        uint8_t* p = (uint8_t*)(memccpy(nextWrite+1, nextRead, ILLEGAL_CHAR, size - bytesCopied));
        uint8_t thisBytesCopied = ((p == nullptr) ? (size - bytesCopied + 1) : (p - nextWrite - 1));

        *(nextWrite) = thisBytesCopied;
        nextWrite += thisBytesCopied;
        bytesCopied += thisBytesCopied;
        nextRead += thisBytesCopied;
    }

    if(*(nextWrite) == ILLEGAL_CHAR){
        *(nextWrite++) = '\x01';
    }
    *(nextWrite) = '\x00';
}

void VexSerial::deserializeMsg(const uint8_t* const ser_msg, uint8_t* dst, const uint8_t size){

    const uint8_t* nextRead = ser_msg;
    uint8_t* nextWrite = dst;

    uint8_t amtToCpy = *(nextRead++);
    while(amtToCpy != 0){
        memcpy(nextWrite, nextRead, amtToCpy-1);
        nextWrite += amtToCpy-1;
        nextRead += amtToCpy-1;

        *(nextWrite++) = ILLEGAL_CHAR;

        amtToCpy = *(nextRead++);
    }

    //add null terminator
    nextWrite--;
    *nextWrite = '\x00';
}

// TODO - techincally a 2-copy implementation
//  could get down to a 1-copy if needed
void VexSerial::receiveMessage(uint8_t* const dst, uint8_t& size){
    uint8_t recvBuffer[STREAM_SZ_REQUIRED];

    uint8_t chunkSize;
    uint8_t* nextWrite = recvBuffer;
    
    read(VexSerial::SerialFD, nextWrite++, 1);

    while((chunkSize = *(nextWrite-1)) != 0){
        read(VexSerial::SerialFD, nextWrite, chunkSize);
        nextWrite += chunkSize;
    }

    const uint8_t messageLen = nextWrite - recvBuffer;
    deserializeMsg(recvBuffer, dst, messageLen);
    size = messageLen - 2;
}