#ifdef NOT_PROS
#include "../../CPP/ThreadQueue.h"
#include <stdexcept>
#include <thread>
#include <chrono>

#define TIMEOUT_MAX ((uint32_t)0xffffffffUL)
#else
#include "main.h"
#endif

#define RETRY_FREQUENCY_MS 50

#include "vexSerial.h"

class VexMessenger
{
private:
    struct MessageHeader
    {
        uint8_t len;
        uint8_t csum; // unused
        uint8_t msgID; // unused
        uint8_t msgType;
    };

    struct Message
    {
        VexMessenger::MessageHeader header;
        uint8_t data[MAX_MESSAGE_LEN - sizeof(VexMessenger::MessageHeader)];

        inline Message(void)
        {
            memset(this, 0, sizeof(Message));
        }
    };

    enum MessageTypes
    {
        MESSAGE_TYPE_DATA = 0,
        MESSAGE_TYPE_HELLO = 1,
        MESSAGE_TYPE_HELLO_ACK = 2,
        MESSAGE_TYPE_GOODBYE = 3,
        MESSAGE_TYPE_GOODBYE_ACK = 4,
        MESSAGE_TYPE_ECHO = 5,
        MESSAGE_TYPE_ECHO_ACK = 6
    };

    static volatile bool is_connected;

#ifdef NOT_PROS
    static ThreadQueue q;

    //TODO - some thread bs here
#else
    static const pros::c::queue_t AvailablePool;
    static const pros::c::queue_t ReceivePool;

    //used for the damn queue
    static Message messagePool[MAX_MESSAGES_IN_FLIGHT];

    pros::Task recvTask;
#endif

    static void MessengerReceiver(void* params);

    VexMessenger(void);
    ~VexMessenger();

    static void handle_control(VexMessenger::Message * const msg);

    static inline void send_message(Message const *const msg){
        VexSerial::sendMessage((uint8_t const * const)(msg), msg->header.len);
    };
public:
    static VexMessenger *const v_messenger;

    class UnexpectedDisconnection : public std::exception {};

    VexMessenger(VexMessenger const &) = delete;
    void operator=(VexMessenger const &) = delete;

    inline bool isConnected(void) { return is_connected; }

    // send a data message to the other side
    //  may throw UnexpectedDisconnection if disconnected
    inline void sendMessage(uint8_t const * const buff, uint8_t &len)
    {
        VexMessenger::Message out_msg;
        out_msg.header.len = len + sizeof(VexMessenger::MessageHeader);
        out_msg.header.msgType = static_cast<uint8_t>(VexMessenger::MessageTypes::MESSAGE_TYPE_DATA);
        send_message(&out_msg);
    }

    // attempt a connection, return true if connection established
    bool tryConnect(uint32_t const timeout_ms = 1000);

    // blocking until connected
    inline void connect(void){
        tryConnect(TIMEOUT_MAX);
    }

    //TODO - disconnection
};


// class VexMessenger
// {
// private:

// public:

//     


//     // attempt a connection, return true if connection established
//     inline bool try_connect(uint32_t const timeout_ms = 1000){
//         return try_cycle(timeout_ms, true);
//     }

//     // attempt a disconnect, return true if connection is now disconnected
//     //  any pending messages will be destroyed
//     inline bool try_disconnect(uint32_t const timeout_ms = 1000){
//         return try_cycle(timeout_ms, false);
//     }

//     // blocking until connected
//     inline void connect(void){
//         try_connect(TIMEOUT_MAX);
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


//     // NOT Implemented
//     // set the callback function for echo-ack messages
//     // void setEchoAckCallback(function_ptr);

//     // NOT Implemented
//     // clear the callback function for echo-ack messages
//     // void clearEchoAckCallback(void);
// };
