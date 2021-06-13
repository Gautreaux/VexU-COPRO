#ifndef __VEX_MESSENGER__
#define __VEX_MESSENGER__

#include <stdexcept>

#include "vexSerial.h"

#define TIMEOUT_MAX (-1u)

//for tryConnect/tryDisconnect, how frequently to retry
#define RETRY_INTERVAL_MS 200u

namespace VexMessenger {
    class UnexpectedDisconnection : public std::exception
    {
    };

    // start up the messenger
    //  must call once
    //  should probably call only once
    void initMessenger(void);

    // return true iff connected
    bool isConnected(void);

    // send buffer as a data message
    void sendMessage(uint8_t const * const buff, uint8_t len);

    // try and connect,
    //   return true if connected before timeout
    //      else return false
    bool tryConnect(uint32_t const timeout_ms = 1000);

    // blocking until connected
    inline void connect(void)
    {
        tryConnect(TIMEOUT_MAX);
    }

    // read messages until a data message
    //  returns true if a message was successfully read
    //  returns false if a disconnect occurs before timeout
    //  returns false if a timeout occurs before the next data message
    //  immediately returns false if disconnected
    bool readDataMessage(uint8_t * const buff, uint8_t& len, uint32_t const timeout_ms);

    // read messages until a data message
    //  will throw UnexpectedDisconnection if disconnected data message
    //    or if called while disconnected
    void readDataMessageBlocking(uint8_t * const buff, uint8_t& len);
};

#endif //__VEX_MESSENGER__

// class VexMessenger
// {
// private:
    
    // // returns true if a message was successfully received in time
    // inline bool receive_message(Message * const msg, uint32_t const timeout_ms = TIMEOUT_MAX){
    //     uint8_t msgLen;
    //     //could do some assert that msgLen == msg->header.len
    //     return VexSerial::v_ser->receiveMessage((uint8_t * const)(msg), msgLen, timeout_ms);
    // }

    // // spin in a cycle for connect/disconnect
    // bool try_cycle(uint32_t const timeout_ms, bool const isConnect);

// public:

//     // attempt a connection, return true if connection established
//     inline bool try_connect(uint32_t const timeout_ms = 1000){
//         return try_cycle(timeout_ms, true);
//     }

//     // attempt a disconnect, return true if connection is now disconnected
//     //  any pending messages will be destroyed
//     inline bool try_disconnect(uint32_t const timeout_ms = 1000){
//         return try_cycle(timeout_ms, false);
//     }



//     // blocking until clean disconnect
//     //  any pending messages will be destroyed
//     inline void disconnect(void){
//         try_disconnect(TIMEOUT_MAX);
//     }

//     // read messages until a data message
//     //  returns true if a message was successfully read
//     //  returns false if a disconnect occurs before timeout
//     //  returns false if a timeout occurs before the next data message
//     //  immediately returns false if disconnected
//     bool readDataMessage(uint8_t * const buff, uint8_t& len, uint32_t const timeout_ms);

//     // read messages until a data message
//     //  will throw UnexpectedDisconnection if disconnected data message
//     //    or if called while disconnected
//     void readDataMessageBlocking(uint8_t * const buff, uint8_t& len);

//     inline bool readIfAvailable(uint8_t * const buff, uint8_t& len){
//         return readDataMessage(buff, len, 0);
//     }

    
//     // send a data message to the other side
//     //  may throw UnexpectedDisconnection if disconnected
//     inline void sendMessage(uint8_t const * const buff, uint8_t &len)


//     // NOT Implemented
//     // set the callback function for echo-ack messages
//     // void setEchoAckCallback(function_ptr);

//     // NOT Implemented
//     // clear the callback function for echo-ack messages
//     // void clearEchoAckCallback(void);
// };