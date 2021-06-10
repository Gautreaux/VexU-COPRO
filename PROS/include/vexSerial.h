#ifdef NOT_PROS
#include <string.h>
#include <stdint.h>
#include <cstdio>
#else
#include "main.h"
#endif

#define MAX_MESSAGE_LEN 100
#define STREAM_SZ_REQUIRED (MAX_MESSAGE_LEN + 2)
#define MAX_MESSAGES_IN_FLIGHT 10
#define ILLEGAL_CHAR 'p'

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
    // TODO - will this need to be adapted for non-pros operation?
    fwrite(msg, size, 1, stdout);
    fflush(stdout);
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

// class VexSerial {
// private:
//     struct PendingMessage
//     {
//         //message len including padding
//         uint8_t messageLen;

//         uint8_t messagebuff[MAX_MESSAGE_LEN+1];
//         //+1 for safety null character

//         PendingMessage(void);
//     };

//     struct VexSerialQueues
//     {
//         const pros::c::queue_t AvailablePool;
//         const pros::c::queue_t SendingPool;
//         const pros::c::queue_t ReceivePool; 
//         bool Running;   

//         VexSerialQueues(pros::c::queue_t a, pros::c::queue_t s, pros::c::queue_t r);
//         ~VexSerialQueues(void);
//     };

//     static PendingMessage messagePool[MAX_MESSAGES_IN_FLIGHT];

//     static const VexSerialQueues vsq;

//     //tasks for sending and receiving
//     pros::Task sendTask;
//     pros::Task recvTask;



//     //DO NOT CALL DIRECTLY
//     static void VexSerialSender(void* params);
//     static void VexSerialReceiver(void* params);

//     // inline static void VexSerialSender(void* params){
//     //     uint64_t ctr = 0;
//     //     while (true)
//     //     {
//     //         pros::lcd::print(2, "Sender %d", ctr);
//     //         ctr++;
//     //         pros::delay(50);
//     //     }
//     // }
//     // inline static void VexSerialReceiver(void* params){
//     //     uint64_t ctr = 0;
//     //     while (true)
//     //     {
//     //         pros::lcd::print(3, "Receiver %d", ctr);
//     //         ctr++;
//     //         pros::delay(30);
//     //     }
//     // }

//     static void setup(void);

//     VexSerial(void);
//     ~VexSerial();
// public:
//     static VexSerial* const v_ser;

//     VexSerial(VexSerial const&) = delete;
//     void operator=(VexSerial const&) = delete;

//     //enqueue a message to be sent
//     //  message len <= MAX_MESSAGE_LEN
//     //  len does not include null terminator and 
//     //      one is not needed
//     void sendMessage(const uint8_t* const msg, const uint8_t size);

//     //receive a message of len <= MAX
//     //  returns true if recv succeeded, 
//     //      false if timeout was exceeded
//     //  timeout = 0 --> receive if avaliable
//     //  TIMEOUT_MAX --> wait forever
//     bool receiveMessage(uint8_t* const recv_out, uint8_t& size_out, uint32_t timeout = TIMEOUT_MAX);

//     inline bool receiveMessageIfAvailable(uint8_t* const recv_out, uint8_t& size_out){
//         return receiveMessage(recv_out, size_out, 0);
//     }
// };
