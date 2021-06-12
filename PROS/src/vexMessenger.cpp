#ifdef NOT_PROS
#include "../include/vexMessenger.h"
#else
#include "vexMessenger.h"
#endif

volatile bool VexMessenger::is_connected = false;

#ifdef NOT_PROS
VexMessenger::VexMessenger(void) :
recvThread(VexMessenger::MessengerReceiver, nullptr)
{
    printf("VEX Messenger Started\n");
    // TODO - launch async reciever,
    int serialFD = open(SERIAL_FILE_PATH, O_RDONLY);
    if(serialFD < 0){
        printf("Error %d occurred whic opening serial file.\n", serialFD);
    }
    // make the file non-blocking
    // fcntl(serialFD, F_SETFL, O_NONBLOCK);
    VexSerial::SerialFD = serialFD;
}

VexMessenger::~VexMessenger(void)
{
    // TODO - try and disconnect cleanly
    // if(isConnected()){
    //     try_disconnect(200);
    // }
}
#else
VexMessenger::VexMessenger(void) : 
recvTask(MessengerReceiver, nullptr, "recvTask")
{
    pros::c::serctl(SERCTL_DISABLE_COBS, NULL);

    for(unsigned int i = 0; i < MAX_MESSAGES_IN_FLIGHT; i++){
        VexMessenger::Message* thisMsg = messagePool + i;
        pros::c::queue_append(AvailablePool, &thisMsg, TIMEOUT_MAX);
    }    
}

VexMessenger::~VexMessenger(void)
{
    // TODO - try and disconnect cleanly
    // if(isConnected()){
    //     try_disconnect(200);
    // }

    pros::c::queue_delete(AvailablePool);
    pros::c::queue_delete(ReceivePool);
}
#endif

// loops forever in own task/thread reading messages
void VexMessenger::MessengerReceiver(void* params){

#ifdef DEBUG_NOT_PROS
    printf("Receiver Started\n");
#endif
    VexMessenger::Message response;
    uint8_t messageSize;

    while(true){
        VexSerial::receiveMessage((uint8_t*)(&response), messageSize);

#ifdef DEBUG_NOT_PROS
        printf("Received new message of len %d : header [%02X %02X %02X %02X]\n",
            messageSize, response.header.len, response.header.csum, response.header.msgID, response.header.msgType
        );
#endif

        if(response.header.msgType == VexMessenger::MessageTypes::MESSAGE_TYPE_DATA){
            if(VexMessenger::v_messenger->isConnected()){
                //destroy any data messages before connection
                continue;
            }

            VexMessenger::Message* thisMsg;
#ifdef NOT_PROS
            if(AvailablePool.get(&thisMsg, TIMEOUT_MAX)){
                memcpy(thisMsg, &response, sizeof(VexMessenger::Message));
                ReceivePool.put(&thisMsg, TIMEOUT_MAX);
            }else{
                //some error
            }
#else

            if(pros::c::queue_recv(AvailablePool, &thisMsg, TIMEOUT_MAX)){
                memcpy(thisMsg, &response, sizeof(VexMessenger::Message));
                pros::c::queue_append(ReceivePool, &thisMsg, TIMEOUT_MAX);
            }else{
                //some error
            }
#endif
        }else{
            handle_control(&response);
        }
    }
}

void VexMessenger::handle_control(VexMessenger::Message * const msg){
    switch (msg->header.msgType){
        case VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO:
            msg->header.msgType = static_cast<uint8_t>(VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO_ACK);
            send_message(msg);
            is_connected = true;
#ifdef DEBUG_NOT_PROS
            printf("Messenger connected\n");
#endif
            break;
        case VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO_ACK:
            is_connected = true;
#ifdef DEBUG_NOT_PROS
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
#ifdef NOT_PROS
            printf("handle_control illegal control: %d", msg->header.msgType);
#else
            pros::lcd::print(6, "handle_control illegal control: %d", msg->header.msgType);
#endif
            throw msg->header.msgType;
    }
}

bool VexMessenger::tryConnect(uint32_t const timeout_ms){
    if(isConnected()){
        return true;
    }

    const uint32_t retryTime = std::min(RETRY_FREQUENCY_MS, (int)timeout_ms);
    uint32_t elapsed = 0;

    do{
        VexMessenger::Message msg;
        msg.header.len = sizeof(MessageHeader);
        msg.header.msgType = VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO;
        send_message(&msg);

#ifdef NOT_PROS
        std::this_thread::sleep_for(std::chrono::milliseconds(retryTime));
#else
        pros::delay(retryTime);
#endif
        if(isConnected()){
            return true;
        }

        elapsed += retryTime;
    }while(elapsed < timeout_ms);

    return false;
}

//TODO - there is a way to convert this to a no-copy operation
bool VexMessenger::readDataMessage(uint8_t * const buff, uint8_t& len, uint32_t const timeout_ms){
    VexMessenger::Message* msg;

    if(!isConnected()){
        throw UnexpectedDisconnection();
    }

#ifdef NOT_PROS
    // TODO
    if(ReceivePool.get(&msg, timeout_ms)){
#else
    if(pros::c::queue_recv(ReceivePool, &msg, timeout_ms)){
#endif
        len = msg->header.len - sizeof(VexMessenger::MessageHeader);
        memcpy(buff, msg->data, len);
#ifdef NOT_PROS
        AvailablePool.put(&msg, TIMEOUT_MAX);
#else
        pros::c::queue_append(AvailablePool, &msg, TIMEOUT_MAX);
#endif
        return true;
    }
    // either a disconnect or a timeout
    return false;
}

// ThreadQueue<VexMessenger::Message*, MAX_MESSAGES_IN_FLIGHT> VexMessenger::q();
#ifdef NOT_PROS
VexMessenger::TQ VexMessenger::AvailablePool;
VexMessenger::TQ VexMessenger::ReceivePool;
#else
pros::c::queue_t const VexMessenger::AvailablePool(pros::c::queue_create(MAX_MESSAGES_IN_FLIGHT, sizeof(VexMessenger::Message*)));
pros::c::queue_t const VexMessenger::ReceivePool(pros::c::queue_create(MAX_MESSAGES_IN_FLIGHT, sizeof(VexMessenger::Message*)));
#endif
VexMessenger::Message VexMessenger::messagePool[MAX_MESSAGES_IN_FLIGHT];
VexMessenger * const VexMessenger::v_messenger = new VexMessenger();