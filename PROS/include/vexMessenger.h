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
    static ThreadQueue<VexMessenger::Message*, MAX_MESSAGES_IN_FLIGHT> q;

    //TODO - some thread bs here
#else
    static const pros::c::queue_t AvailablePool;
    static const pros::c::queue_t ReceivePool;

    pros::Task recvTask;
#endif
    static Message messagePool[MAX_MESSAGES_IN_FLIGHT];

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

  // read messages until a data message
    //  returns true if a message was successfully read
    //  returns false if a disconnect occurs before timeout
    //  returns false if a timeout occurs before the next data message
    bool readDataMessage(uint8_t * const buff, uint8_t& len, uint32_t const timeout_ms);

    //TODO - disconnection
};