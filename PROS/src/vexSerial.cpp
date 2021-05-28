#include "vexSerial.h"

void VexSerial::sendHello(){
    fprintf(stdout, "\x00\x00");
}

void VexSerial::sendHelloAck(){
    fprintf(stdout, "\x00\x01");
}

void VexSerial::sendGoodbye(){
    fprintf(stdout, "\x00\x09");
    taskOk = false;
}

void VexSerial::sendGoodbyeAck(){
    fprintf(stdout, "\x00\x0A");
    taskOk = false;
}

VexSerial::VexSerial(void): taskOk(true), receiveTask(receiveDataWrapper), callback(NULL)
{
    //setup the serial outputs in prose
    //pros::c::serctl(DEVCTL_SET_BAUDRATE, 115200);
	pros::c::serctl(SERCTL_DISABLE_COBS, NULL);

    //setup the out stream buffer
    memset(strBuff, 0, STREAM_BUFFER_SZ);
    setbuffer(stdout, strBuff, STREAM_BUFFER_SZ);

    //setup task
    // pros::newTask = pros::Task(receiveData, NULL);
    // receiveTask = std::move(newTask);

    //try and connect to the friend
    sendHello();
}

VexSerial::~VexSerial(){
    //TODO - cleanup task properly
}

void VexSerial::receiveData() {
    uint8_t c;
    uint8_t body[STREAM_BUFFER_SZ];
    memset(body, 0, STREAM_BUFFER_SZ);

    while(taskOk){
        fgets((char*)(&c), 1, stdin);
        if(c == 0){
            //control operation
            receiveControl();
        }
        else{
            fgets((char*)body, c, stdin);
            if(callback != NULL){
                callback(body, c);
            }
        }
    }
}

void VexSerial::receiveControl(){
    uint8_t c;
    fgets((char*)(&c), 1, stdin);
    switch (c)
    {
    case 0:
        //received hello
        sendHelloAck();
        break;
    case 1:
        //received hello-ack
        break;
    case 9:
        //received goodbye
        sendGoodbyeAck();
        break;
    case 10:
        //received goodbye ack
        break;
    default:
        break;
    }
}

void VexSerial::sendData(const uint8_t* const buff, const size_t size){
    if(size == 0){
        return;
    }

    size_t offset = 0;
    uint8_t buffer[STREAM_BUFFER_SZ];
    memset(buffer, 0, STREAM_BUFFER_SZ);

    while(offset < size){
        uint8_t thisSize = (uint8_t)(std::min(((unsigned int)STREAM_BUFFER_SZ - 1), size - offset));
        buffer[0] = thisSize;
        memcpy(buffer + 1, buff + offset, thisSize);
        fwrite(buffer, 1, thisSize, stdout);
        fflush(stdout);
        offset += thisSize;
    }
}

VexSerial * const VexSerial::v_ser = new VexSerial();

void receiveDataWrapper(void){
    return VexSerial::v_ser->receiveData();
}