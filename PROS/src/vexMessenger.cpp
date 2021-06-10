#ifdef NOT_PROS
#include "../include/vexMessenger.h"
#else
#include "vexMessenger.h"
#endif

VexMessenger::VexMessenger(void) : is_connected(false)
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


// bool VexMessenger::try_cycle(uint32_t const timeout_ms, bool const isConnect){
//     uint8_t const out = static_cast<uint8_t>((isConnect) ? ((VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO)) : (VexMessenger::MessageTypes::MESSAGE_TYPE_GOODBYE));
//     uint8_t const in = static_cast<uint8_t>((isConnect) ? ((VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO_ACK)) : (VexMessenger::MessageTypes::MESSAGE_TYPE_GOODBYE_ACK));

//     VexMessenger::Message out_message;
//     out_message.header.len = sizeof(VexMessenger::MessageHeader);
//     out_message.header.msgType = out;

//     VexMessenger::Message response_message;

//     uint32_t timeRemaining = timeout_ms;
//     uint32_t const startTime = pros::millis();

//     while(isConnected() != isConnect){
//         send_message(&out_message); // potentially a little spammy
//         if(receive_message(&response_message, timeRemaining)){
//             if(response_message.header.msgType == in){
//                 is_connected = isConnect;
//                 return true;
//             }
//             else if(response_message.header.msgType == out){
//                 // the client hello'ed us, we must respond
//                 out_message.header.len = sizeof(VexMessenger::MessageHeader);
//                 out_message.header.msgType = in;
//                 send_message(&out_message);
//                 is_connected = isConnect;
//                 return true;
//             }else{
//                 // received a message before the response
//                 // probably just noise in the buffer
//                 //  just discard it
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
//     return true;
// }

// void VexMessenger::handle_control(VexMessenger::Message * const msg){
//     switch (msg->header.msgType){
//         case VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO:
//             msg->header.msgType = static_cast<uint8_t>(VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO_ACK);
//             send_message(msg);
//             is_connected = true;
//             break;
//         case VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO_ACK:
//             is_connected = true;
//             break;
//         case VexMessenger::MessageTypes::MESSAGE_TYPE_GOODBYE:
//             msg->header.msgType = static_cast<uint8_t>(VexMessenger::MessageTypes::MESSAGE_TYPE_GOODBYE_ACK);
//             send_message(msg);
//             break;
//         case VexMessenger::MessageTypes::MESSAGE_TYPE_GOODBYE_ACK:
//             is_connected = false;
//             break;
//         case VexMessenger::MessageTypes::MESSAGE_TYPE_ECHO:
//             msg->header.msgType = static_cast<uint8_t>(VexMessenger::MessageTypes::MESSAGE_TYPE_ECHO_ACK);
//             send_message(msg);
//             break;
//         case VexMessenger::MessageTypes::MESSAGE_TYPE_ECHO_ACK:
//             //TODO - probably want to do something here w.r.t processing but whatever
//             break;
//         default:
//             pros::lcd::print(6, "handle_control illegal control: %d", msg->header.msgType);
//             throw msg->header.msgType;
//     }
// }

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

// VexMessenger * const VexMessenger::v_messenger = new VexMessenger();