#ifdef NOT_PROS
#include "../include/vexMessenger.h"
#else
#include "vexMessenger.h"
#endif

volatile bool VexMessenger::is_connected = false;

#ifdef NOT_PROS
VexMessenger::VexMessenger(void)
{
    // TODO - launch async reciever,
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
        VexSerial::Message* thisMsg = messagePool + i;
        pros::c::queue_append(vsq.AvailablePool, &thisMsg, TIMEOUT_MAX);
    }    
}

VexMessenger::~VexMessenger(void)
{
    // TODO - try and disconnect cleanly
    // if(isConnected()){
    //     try_disconnect(200);
    // }

    pros::c::queue_delete(q);
}
#endif

// loops forever in own task/thread reading messages
void VexMessenger::MessengerReceiver(void* params){
    VexMessenger::Message response;
    uint8_t messageSize;

    while(true){
        VexSerial::receiveMessage((uint8_t*)(&response), messageSize);

        if(response.header.msgType == VexMessenger::MessageTypes::MESSAGE_TYPE_DATA){
#ifdef NOT_PROS
            //TODO - q.append
#else
            VexMessenger::Message* thisMsg;

            if(pros::c::queue_recv(AvailablePool, &thisMessage, TIMEOUT_MAX)){
                memcpy(thisMsg, &response, sizeof(VexMessenger::Message));
                pros::c::queue_append(ReceivePool, &thisMsg, TIMEOUT_MAX);
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
            break;
        case VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO_ACK:
            is_connected = true;
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
        msg.header.len = 4;
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

// bool VexMessenger::readDataMessage(uint8_t * const buff, uint8_t& len, uint32_t const timeout_ms){
//     VexMessenger::Message response;

//     uint32_t timeRemaining = timeout_ms;
//     uint32_t startTime = pros::millis();

//     while(isConnected()){
//         if(receive_message(&response, timeRemaining)){
//             if(response.header.msgType == VexMessenger::MessageTypes::MESSAGE_TYPE_DATA){
//                 len = response.header.len - sizeof(VexMessenger::MessageHeader);
//                 memcpy(buff, response.data, len);
//                 return true;
//             }else{
//                 // some form of a control message that we need to process
//                 handle_control(&response);

//                 //and update time remaining
//                 timeRemaining = timeout_ms - (pros::millis() - startTime);
//                 if(timeRemaining > timeout_ms){
//                     //under flow occurred
//                     return false;
//                 }
//             }
//         }else{
//             //timeout
//             return false;
//         }
//     }

//     return false;
// }

// void VexMessenger::readDataMessageBlocking(uint8_t * const buff, uint8_t& len){
//     bool b = readDataMessage(buff, len, TIMEOUT_MAX);
//     if(b){
//         return;
//     }

//     throw UnexpectedDisconnection();
// }
#ifdef NOT_PROS
#else
pros::c::queue_t const VexMessenger::AvailablePool(pros::c::queue_create(MAX_MESSAGES_IN_FLIGHT, sizeof(VexMessenger::Message*))),
pros::c::queue_t const VexMessenger::ReceivePool(pros::c::queue_create(MAX_MESSAGES_IN_FLIGHT, sizeof(VexMessenger::Message*))),
#endif
VexMessenger * const VexMessenger::v_messenger = new VexMessenger();