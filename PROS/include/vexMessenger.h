#include "main.h"
#include "vexSerial.h"

class VexMessenger
{
public:
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
        MESSAGE_TYPE_GOODBYE_ACK = 4
    };

private:
    bool is_connected;

    VexMessenger(void);
    ~VexMessenger();

    inline void send_message(Message const *const msg){
        VexSerial::v_ser->sendMessage((uint8_t const * const)(msg), msg->header.len);
    };
    
    // returns true if a message was successfully received in time
    inline bool receive_message(Message const * const msg, uint32_t const timeout_ms = TIMEOUT_MAX){
        uint8_t msgLen;
        //could do some assert that msgLen == msg->header.len
        return VexSerial::v_ser->receiveMessage((uint8_t * const)(msg), msgLen, timeout_ms);
    }

    // spin in a cycle for connect/disconnect
    bool try_cycle(uint32_t const timeout_ms, bool const isConnect);

public:
    static VexMessenger *const v_messenger;

    VexMessenger(VexMessenger const &) = delete;
    void operator=(VexMessenger const &) = delete;

    inline bool isConnected(void) { return is_connected; }

    // attempt a connection, return true if connection established
    inline bool try_connect(uint32_t const timeout_ms = 1000){
        return try_cycle(timeout_ms, true);
    }

    // attempt a disconnect, return true if connection is now disconnected
    //  any pending messages will be destroyed
    inline bool try_disconnect(uint32_t const timeout_ms = 1000){
        return try_cycle(timeout_ms, false);
    }

    // blocking until connected
    inline void connect(void){
        try_connect(TIMEOUT_MAX);
    }

    // blocking until clean disconnect
    //  any pending messages will be destroyed
    inline void disconnect(void){
        try_disconnect(TIMEOUT_MAX);
    }
};
