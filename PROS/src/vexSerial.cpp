#include "vexSerial.h"

VexSerial::PendingMessage::PendingMessage(void){
    memset(this, 0, sizeof(PendingMessage));
}

VexSerial::VexSerialQueues::VexSerialQueues(
    pros::c::queue_t a, pros::c::queue_t s, pros::c::queue_t r
) : AvailablePool(a), SendingPool(s), ReceivePool(r), Running(true)
{}

VexSerial::VexSerialQueues::~VexSerialQueues(void){
    pros::c::queue_delete(AvailablePool);
    pros::c::queue_delete(SendingPool);
    pros::c::queue_delete(ReceivePool);
}

VexSerial::VexSerial(void) : 
    sendTask(VexSerialSender, (void *)(&vsq), "sendTask"),
    recvTask(VexSerialReceiver, (void *)(&vsq), "readTask")
{
    //setup the serial outputs in prose
    //pros::c::serctl(DEVCTL_SET_BAUDRATE, 115200);
    pros::c::serctl(SERCTL_DISABLE_COBS, NULL);

    setup();
}

VexSerial::~VexSerial(){
    //TODO : teardown tasks properly
}

void VexSerial::setup(void){
    for(unsigned int i = 0; i < MAX_MESSAGES_IN_FLIGHT; i++){
        VexSerial::PendingMessage* thisMsg = messagePool + i;
        pros::c::queue_append(vsq.AvailablePool, &thisMsg, TIMEOUT_MAX);
    }    
}

void VexSerial::serializeMsg(const uint8_t* const msg, uint8_t* dst, const uint8_t size){

    const uint8_t* nextRead = msg;
    uint8_t* nextWrite = dst;
    uint8_t bytesCopied = 0;

    while(bytesCopied < size){
        uint8_t* p = (uint8_t*)(memccpy(nextWrite+1, nextRead, ILLEGAL_CHAR, size - bytesCopied));
        uint8_t thisBytesCopied = ((p == nullptr) ? (size - bytesCopied + 1) : (p - nextWrite - 1));

        *(nextWrite) = thisBytesCopied;
        nextWrite += thisBytesCopied;
        bytesCopied += thisBytesCopied;
        nextRead += thisBytesCopied;
    }

    if(*(nextWrite) == ILLEGAL_CHAR){
        *(nextWrite++) = '\x01';
    }
    *(nextWrite) = '\x00';
}

void VexSerial::deserializeMsg(const uint8_t* const ser_msg, uint8_t* dst, const uint8_t size){

    pros::lcd::print(6, "DER LINE: %d", __LINE__);
    pros::delay(750);
    pros::lcd::print(6, "src ptr: %p", ser_msg);
    pros::delay(2000);
    pros::lcd::print(6, "dst ptr: %p", dst);
    pros::delay(2000);

    const uint8_t* nextRead = ser_msg;
    uint8_t* nextWrite = dst;

    pros::lcd::print(6, "DER LINE: %d", __LINE__);
    pros::delay(750);
    uint8_t amtToCpy = *(nextRead++);
    pros::lcd::print(6, "DER LINE: %d", __LINE__);
    pros::delay(750);
    while(amtToCpy != 0){
    pros::lcd::print(6, "DER LINE: %d", __LINE__);
    pros::delay(750);
    pros::lcd::print(6, "Write ptr: %p", nextWrite);
    pros::delay(2000);
    pros::lcd::print(6, "Read ptr: %p", nextRead);
    pros::delay(2000);
    pros::lcd::print(6, "Write amt: %d", amtToCpy);
    pros::delay(2000);
        memcpy(nextWrite, nextRead, amtToCpy-1);
    pros::lcd::print(6, "DER LINE: %d", __LINE__);
    pros::delay(750);
        nextWrite += amtToCpy-1;
        nextRead += amtToCpy-1;

    pros::lcd::print(6, "DER LINE: %d", __LINE__);
    pros::delay(750);
        *(nextWrite++) = ILLEGAL_CHAR;
    pros::lcd::print(6, "DER LINE: %d", __LINE__);
    pros::delay(750);

        amtToCpy = *(nextRead++);
    }
    pros::lcd::print(6, "DER LINE: %d", __LINE__);
    pros::delay(750);

    //add null terminator
    nextWrite--;
    *nextWrite = '\x00';
    pros::lcd::print(6, "DER LINE: %d", __LINE__);
    pros::delay(750);
}

void VexSerial::VexSerialSender(void* params){
    // VexSerialQueues* const vsq = (VexSerialQueues*)(params);

    uint8_t formatBuffer[MAX_MESSAGE_LEN];

    pros::delay(2000);
    pros::lcd::print(2, "Sender Started");


    pros::delay(1000);
    pros::lcd::print(7, "Sendln: %d", __LINE__);
    pros::delay(1000);


    while(vsq.Running){
    pros::lcd::print(7, "Sendln: %d", __LINE__);
    pros::delay(1000);
        PendingMessage* thisMessage;
    pros::lcd::print(7, "Sendln: %d", __LINE__);
    pros::delay(1000);

        if(pros::c::queue_recv(vsq.SendingPool, &thisMessage, TIMEOUT_MAX)){
    pros::lcd::print(7, "Sendln: %d", __LINE__);
    pros::delay(1000);
            //format the message to remove all 'p'
            serializeMsg(thisMessage->messagebuff, formatBuffer, thisMessage->messageLen);
    pros::lcd::print(7, "Sendln: %d", __LINE__);
    pros::delay(1000);

            //write the formatted message
            fwrite(formatBuffer, thisMessage->messageLen + 2, 1, stdout);
    pros::lcd::print(7, "Sendln: %d", __LINE__);
    pros::delay(1000);
            fflush(stdout);
    pros::lcd::print(7, "Sendln: %d", __LINE__);
    pros::delay(1000);

            memset(thisMessage, 0, sizeof(PendingMessage));

            //should never block
            pros::c::queue_append(vsq.AvailablePool, &thisMessage, TIMEOUT_MAX);
        }else{
            // the recv was cancelled
            // the program is probably tearing down
        }
    }
}

void VexSerial::VexSerialReceiver(void* params){
    // VexSerialQueues* const vsq = (VexSerialQueues*)(params);

    uint8_t formatBuffer[MAX_MESSAGE_LEN];

    pros::delay(1000);
    pros::lcd::print(3, "Receiver Started");

    while(vsq.Running){

        uint8_t chunkSize;
        uint8_t messageLen = 0;
        uint8_t* nextWrite = formatBuffer;
        
        pros::lcd::print(6, "READ LINE: %d", __LINE__);

        *(nextWrite++) = fgetc(stdin);
        while((chunkSize = *(nextWrite-1)) != 0){
            pros::lcd::print(3, "new chunk size: %d", chunkSize);
            fread(nextWrite, 1, chunkSize, stdin);
            nextWrite += chunkSize;
            messageLen += chunkSize + 1;
        }

        pros::lcd::print(3, "MSG: %s", formatBuffer);

        PendingMessage* thisMessage;
        
        pros::lcd::print(6, "READ LINE: %d", __LINE__);
        pros::delay(750);

        if(pros::c::queue_recv(vsq.AvailablePool, &thisMessage, TIMEOUT_MAX)){
            pros::lcd::print(6, "READ LINE: %d", __LINE__);
            pros::delay(750);
            pros::lcd::print(6, "This Message ptr: %p", thisMessage);
            pros::delay(2000);
            pros::lcd::print(6, "This buffer ptr: %p", thisMessage->messagebuff);
            pros::delay(2000);
            deserializeMsg(formatBuffer, thisMessage->messagebuff, messageLen);
            pros::lcd::print(3, "DSER MSG: %s", thisMessage->messagebuff);
            pros::delay(750);
            thisMessage->messageLen = messageLen - 2;
            
            //should never block?
            pros::c::queue_append(vsq.ReceivePool, &thisMessage, TIMEOUT_MAX);
            pros::lcd::print(6, "READ LINE: %d", __LINE__);
            pros::delay(750);
        }else{
            // the recv was cancelled
            // the program is probably tearing down
        }
    }
}

void VexSerial::sendMessage(const uint8_t* const msg, const uint8_t size){
    if(size > MAX_MESSAGE_LEN){
        //TODO - proper exception
        throw "MESSAGE TOO LONG";
    }

    PendingMessage* thisMessage;

    if(pros::c::queue_recv(vsq.AvailablePool, &thisMessage, TIMEOUT_MAX)){
        memcpy(thisMessage->messagebuff, msg, size);
        thisMessage->messageLen = size;

        //should never block
        pros::c::queue_append(vsq.SendingPool, &thisMessage, TIMEOUT_MAX);
    }else{
        // the recv was cancelled
        // the program is probably tearing down
    }
}

bool VexSerial::receiveMessage(uint8_t* const recv_out, uint8_t& size_out, uint32_t timeout){
    PendingMessage* thisMessage;
    if(pros::c::queue_recv(vsq.ReceivePool, &thisMessage, timeout)){
        memcpy(recv_out, thisMessage->messagebuff, thisMessage->messageLen);
        size_out = thisMessage->messageLen;

        //should never block
        pros::c::queue_append(vsq.AvailablePool, &thisMessage, TIMEOUT_MAX);
        return true;
    }else{
        return false;
    }
}

VexSerial::VexSerialQueues const VexSerial::vsq(
    pros::c::queue_create(MAX_MESSAGES_IN_FLIGHT, sizeof(VexSerial::PendingMessage *)),
    pros::c::queue_create(MAX_MESSAGES_IN_FLIGHT, sizeof(VexSerial::PendingMessage *)),
    pros::c::queue_create(MAX_MESSAGES_IN_FLIGHT, sizeof(VexSerial::PendingMessage *))
);

VexSerial::PendingMessage VexSerial::messagePool[MAX_MESSAGES_IN_FLIGHT];

VexSerial * const VexSerial::v_ser = new VexSerial();