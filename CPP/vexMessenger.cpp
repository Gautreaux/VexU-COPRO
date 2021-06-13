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
        
        std::thread recvThread;

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

        // loops forever in own task/thread reading messages
        void MessengerReceiver(void *params)
        {
            VexMessenger::Message response;
            uint8_t messageSize;

            while (true)
            {
                VexSerial::receiveMessage((uint8_t *)(&response), messageSize);

#ifdef DEBUG
                printf("Received message: [%02X %02X %02X %02X] '%s'\n", response.header.len, response.header.csum, response.header.msgID, response.header.msgType, response.data);
#endif

                if (response.header.msgType == VexMessenger::MessageTypes::MESSAGE_TYPE_DATA)
                {
                    if (!isConnected())
                    {
                        //destroy any data messages before connection
                        continue;
                    }

                    VexMessenger::Message *thisMsg;

                    if (availablePool.get(&thisMsg, TIMEOUT_MAX))
                    {
#ifdef DEBUG
                        printf("RecvPool putting %p\n", thisMsg);
#endif
                        memcpy(thisMsg, &response, sizeof(VexMessenger::Message));
                        receivePool.put(&thisMsg, TIMEOUT_MAX);
                    }
                    else
                    {
                        //some error
                    }
                }
                else
                {
                    handle_control(&response);
                }
            }
        }

    }; //anon namespace

    void initMessenger(void){
        is_connected = false;
        for(unsigned int i = 0; i < MAX_MESSAGES_IN_FLIGHT; i++){
            Message* target = (messagePool + i);
            availablePool.put(&target, TIMEOUT_MAX);
        }
        recvThread = std::thread(VexMessenger::MessengerReceiver, nullptr);
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
        if(isConnected()){
            return true;
        }

        const uint32_t retryTime = std::min(RETRY_INTERVAL_MS, timeout_ms);
        uint32_t elapsed_ms = 0;

        VexMessenger::Message msg;
        msg.header.len = sizeof(VexMessenger::MessageHeader);
        msg.header.msgType = VexMessenger::MESSAGE_TYPE_HELLO;
        do {
            send_message(&msg);

            std::this_thread::sleep_for(std::chrono::milliseconds(retryTime));

            if(isConnected()){
                return true;
            }

            elapsed_ms += retryTime;
        }while(elapsed_ms < timeout_ms);
        //timeout
        return false;
    }

    bool readDataMessage(uint8_t * const buff, uint8_t& len, uint32_t const timeout_ms){
        VexMessenger::Message* thisMessage;

        if(receivePool.get(&thisMessage, timeout_ms)){
            len = thisMessage->header.len;
            memcpy(buff, thisMessage->data, len);
            availablePool.put(&thisMessage, TIMEOUT_MAX);
            return true;
        }else{
            //timeout
            return false;
        }
    }

    void readDataMessageBlocking(uint8_t *const buff, uint8_t &len)
    {
        bool b = readDataMessage(buff, len, TIMEOUT_MAX);
        if (b)
        {
            return;
        }

        throw UnexpectedDisconnection();
    }
};
