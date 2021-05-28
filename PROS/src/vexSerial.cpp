#include "vexSerial.h"

void VexSerial::sendHello(){
    fwrite("\x00\x00", 1, 2, stdout);
    fflush(stdout);
}

void VexSerial::sendHelloAck(){
    fwrite("\x00\x01", 1, 2, stdout);
    fflush(stdout);
}

void VexSerial::sendGoodbye(){
    fwrite("\x00\x09", 1, 2, stdout);
    fflush(stdout);
}

void VexSerial::sendGoodbyeAck(){
    fwrite("\x00\x0A", 1, 2, stdout);
    fflush(stdout);
}

VexSerial::VexSerial(void): 
    taskOk(true),
    clientConnected(false),
    callback(NULL),
    receiveTask(receiveDataWrapper, NULL)
{
    //setup the serial outputs in prose
    //pros::c::serctl(DEVCTL_SET_BAUDRATE, 115200);
	pros::c::serctl(SERCTL_DISABLE_COBS, NULL);

    //setup the out stream buffer
    memset(strBuff, 0, STREAM_BUFFER_SZ);
    setbuffer(stdout, strBuff, STREAM_BUFFER_SZ);
}

VexSerial::~VexSerial(){
    //TODO - cleanup task properly
    disconnect();

    //trigger task to exit
    // may dangle wating for message?
    taskOk = false; 
}

void VexSerial::receiveData() {
    uint8_t c[2];
    uint8_t body[STREAM_BUFFER_SZ];
    memset(body, 0, STREAM_BUFFER_SZ);

    while(taskOk){
        fgets((char*)(c), 2, stdin);
        // pros::lcd::print(6, "%02X", c[0]);
        pros::lcd::print(6, "NEW MSG LEN: %d", c[0]);
        pros::delay(500);
        if(c[0] == 0){
            //control operation
            receiveControl();
        }
        if(!stdin){
            pros::lcd::print(6, "stdin error");
            pros::delay(1);
        }
        else{
            memset(body, 0, STREAM_BUFFER_SZ);
            fgets((char*)body, c[0]+1, stdin);
            pros::lcd::print(6, "%02X %02X %02X %02X %02X %02X %02X %02X", body[0], body[1], body[2], body[3], body[4], body[5], body[6], body[7]);
            pros::delay(500);
            if(callback != NULL){
                callback(body, c[0]);
            }
        }
    }
}

void VexSerial::receiveControl(){
    uint8_t c[2];
    fgets((char*)(&c), 2, stdin);
    switch (c[0])
    {
    case 0:
        //received hello
        sendHelloAck();
        clientConnected=true;
        break;
    case 1:
        //received hello-ack
        clientConnected=true;
        break;
    case 9:
        //received goodbye
        sendGoodbyeAck();
        clientConnected=false;
        break;
    case 10:
        //received goodbye ack
        clientConnected=false;
        break;
    default:
        break;
    }
}

void VexSerial::sendData(const uint8_t* const buff, const size_t size){
    if(size == 0){
        return;
    }

    if(clientConnected == false){
        return;
    }

    size_t offset = 0;
    uint8_t buffer[STREAM_BUFFER_SZ];
    memset(buffer, 0, STREAM_BUFFER_SZ);

    while(offset < size){
        uint8_t thisSize = (uint8_t)(std::min(((unsigned int)STREAM_BUFFER_SZ - 1), size - offset));
        buffer[0] = thisSize;
        memcpy(buffer + 1, buff + offset, thisSize);
        pros::lcd::print(4, "%02X %02X %02X %02X %02X %02X %02X %02X", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
        fwrite(buffer, 1, thisSize+1, stdout);
        fflush(stdout);
        offset += thisSize;
    }
}

void VexSerial::tryConnect(void){
    if(clientConnected == false){
        sendHello();
    }
}

void VexSerial::disconnect(void){
    if(clientConnected){
        sendGoodbye();
    }
}

VexSerial * const VexSerial::v_ser = new VexSerial();

void receiveDataWrapper(void* params){
    //return;
    return VexSerial::v_ser->receiveData();
}