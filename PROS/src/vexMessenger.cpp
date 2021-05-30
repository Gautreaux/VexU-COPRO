#include "vexMessenger.h"


VexMessenger::VexMessenger(void) : is_connected(false)
{}

VexMessenger::~VexMessenger(void)
{
    if(isConnected()){
        try_disconnect();
    }
}


bool VexMessenger::try_cycle(uint32_t const timeout_ms, bool const isConnect){
    uint8_t const out = static_cast<uint8_t>((isConnect) ? ((VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO)) : (VexMessenger::MessageTypes::MESSAGE_TYPE_GOODBYE));
    uint8_t const in = static_cast<uint8_t>((isConnect) ? ((VexMessenger::MessageTypes::MESSAGE_TYPE_HELLO_ACK)) : (VexMessenger::MessageTypes::MESSAGE_TYPE_GOODBYE_ACK));

    VexMessenger::Message out_message;
    out_message.header.len = 4;
    out_message.header.msgType = out;

    VexMessenger::Message response_message;

    uint32_t timeRemaining = timeout_ms;
    uint32_t const startMillis = pros::millis();

    while(isConnected() != isConnect){
        send_message(&out_message);
        if(receive_message(&response_message, timeRemaining)){
            if(response_message.header.msgType == in){
                is_connected = isConnect;
                return true;
            }
            else if(response_message.header.msgType == out){
                // the client hello'ed us, we must respond
                out_message.header.len = 4;
                out_message.header.msgType = in;
                send_message(&out_message);
                is_connected = isConnect;
                return true;
            }else{
                // received a message before the response
                // probably just noise in the buffer
                //  just discard it
                timeRemaining = timeout_ms - (pros::millis() - startMillis);
            }
        }else{
            //timeout
            return false;
        }
    }
    return true;
}

VexMessenger * const VexMessenger::v_messenger = new VexMessenger();