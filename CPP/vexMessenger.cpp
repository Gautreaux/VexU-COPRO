#include "vexMessenger.h"

#include <thread>

#include "ThreadQueue.h"

namespace VexMessenger
{
    //private namesapce
    namespace
    {
        struct MessageHeader
        {
            uint8_t len;
            uint8_t csum;  // unused
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

        bool is_connected;
        
        Message messagePool[MAX_MESSAGES_IN_FLIGHT];
        ThreadQueue<Message*, MAX_MESSAGES_IN_FLIGHT> availablePool;
        ThreadQueue<Message*, MAX_MESSAGES_IN_FLIGHT> receivePool;

        // do the sending of a message
        void send_message(Message const *const msg)
        {
            VexSerial::sendMessage((uint8_t const *const)(msg), msg->header.len);
        }

        // handle any message control signals
        //  may alter the contents of msg
        void handle_control(VexMessenger::Message *const msg)
        {
            switch (msg->header.msgType)
            {
            case VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO:
                msg->header.msgType = static_cast<uint8_t>(VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO_ACK);
                send_message(msg);
                is_connected = true;
#ifdef DEBUG
                printf("Messenger connected\n");
#endif
                break;
            case VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO_ACK:
                is_connected = true;
#ifdef DEBUG
                printf("Messenger connected\n");
#endif
                break;
            case VexMessenger::MessageTypes::MESSAGE_TYPE_GOODBYE:
                msg->header.msgType = static_cast<uint8_t>(VexMessenger::MessageTypes::MESSAGE_TYPE_GOODBYE_ACK);
                send_message(msg);
                is_connected = false;
                // stopAll();
                break;
            case VexMessenger::MessageTypes::MESSAGE_TYPE_GOODBYE_ACK:
                is_connected = false;
                // stopAll();
                break;
            case VexMessenger::MessageTypes::MESSAGE_TYPE_ECHO:
                msg->header.msgType = static_cast<uint8_t>(VexMessenger::MessageTypes::MESSAGE_TYPE_ECHO_ACK);
                send_message(msg);
                break;
            case VexMessenger::MessageTypes::MESSAGE_TYPE_ECHO_ACK:
                //TODO - probably want to do something here w.r.t processing but whatever
                break;
            default:
                printf("handle_control illegal control: %d", msg->header.msgType);
                throw msg->header.msgType;
            }
        }

    }; //anon namespace

    class UnexpectedDisconnection : public std::exception
    {
    };

    void initMessenger(void){
        is_connected = false;
        for(unsigned int i = 0; i < MAX_MESSAGES_IN_FLIGHT; i++){
            Message* target = (messagePool + i);
            availablePool.put(&target, TIMEOUT_MAX);
        }
    }

    bool isConnected(void){
        return is_connected;
    }

    void sendMessage(uint8_t const * const buff, const uint8_t len){
        VexMessenger::Message out_msg;
        out_msg.header.len = len + sizeof(VexMessenger::MessageHeader);
        out_msg.header.msgType = static_cast<uint8_t>(VexMessenger::MessageTypes::MESSAGE_TYPE_DATA);
        memcpy(out_msg.data, buff, len);
        send_message(&out_msg);
    }

    bool tryConnect(uint32_t const timeout_ms){
        printf("DANG\n");
        if(isConnected()){
            return true;
        }

        printf("DUNG\n");

        const uint32_t retryTime = std::min(RETRY_INTERVAL_MS, timeout_ms);
        uint32_t elapsed_ms = 0;

        VexMessenger::Message msg;
        msg.header.len = sizeof(VexMessenger::MessageHeader);
        msg.header.msgType = VexMessenger::MESSAGE_TYPE_HELLO;
        do {
            printf("SENDING\n");
            send_message(&msg);

            printf("Sleeping %d ms\n", elapsed_ms);
            std::this_thread::sleep_for(std::chrono::milliseconds(retryTime));
            printf("WOKE\n");

            if(isConnected()){
                return true;
            }

            elapsed_ms += retryTime;
        }while(elapsed_ms < timeout_ms);
        //timeout
        return false;
    }

};
