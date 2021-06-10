#ifdef NOT_PROS
#include "../include/vexSerial.h"
#else
#include "vexSerial.h"
#endif

void serializeMsg(const uint8_t* const msg, uint8_t* dst, const uint8_t size){

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

void deserializeMsg(const uint8_t* const ser_msg, uint8_t* dst, const uint8_t size){

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
