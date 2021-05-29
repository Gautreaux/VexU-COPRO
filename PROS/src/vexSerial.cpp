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

void VexSerial::sendEchoAck(){
    char buffer[STREAM_BUFFER_SZ + 1];
    memset(buffer, 0, STREAM_BUFFER_SZ + 1);

    uint8_t len = fgetc(stdin);

    buffer[0] = '\x00';
    buffer[1] = ECHO_ACK_SIG;
    buffer[2] = len;

    fread(buffer+3, 1, len, stdin);
    fwrite(buffer, 1, len+3, stdout);
    fflush(stdout);
}

void VexSerial::sendSyncAck(){
    pros::lcd::print(7, "ln: %d", __LINE__);
    uint8_t sig = fgetc(stdin);
    pros::lcd::print(7, "ln: %d", __LINE__);
    char buff[4];
    pros::lcd::print(7, "ln: %d", __LINE__);
    buff[0] = '\x00';
    pros::lcd::print(7, "ln: %d", __LINE__);
    buff[1] = SYNC_ACK_MSG;
    pros::lcd::print(7, "ln: %d", __LINE__);
    buff[2] = sig;
    pros::lcd::print(7, "ln: %d", __LINE__);
    buff[3] = '\x00';
    pros::lcd::print(7, "ln: %d", __LINE__);

    pros::lcd::print(6, "Sync acking %02X %d %s", sig, sig, buff+2);
    pros::lcd::print(7, "ln: %d", __LINE__);

    fwrite(buff, 1, 3, stdout);
    pros::lcd::print(7, "ln: %d", __LINE__);
    fflush(stdout);
    pros::lcd::print(7, "ln: %d", __LINE__);
}

VexSerial::VexSerial(void): 
    taskOk(true),
    clientConnected(false),
    callback(NULL),
    receiveTask(receiveDataWrapper, NULL),
    last_connect_attempt_time(time(NULL))
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
    pros::lcd::print(7, "ln: %d", __LINE__);
    uint8_t c;
    pros::lcd::print(7, "ln: %d", __LINE__);
    uint8_t body[STREAM_BUFFER_SZ + 1];
    pros::lcd::print(7, "ln: %d", __LINE__);
    memset(body, 0, STREAM_BUFFER_SZ + 1);
    pros::lcd::print(7, "ln: %d", __LINE__);

    while(taskOk){
    pros::lcd::print(7, "ln: %d", __LINE__);
        c = fgetc(stdin);
    pros::lcd::print(7, "ln: %d", __LINE__);
#ifdef VEX_SERIAL_VERBOSE
        pros::lcd::print(6, "NEW MSG LEN: %d", c);
        pros::delay(500);
#endif
        if(!stdin){
    pros::lcd::print(7, "ln: %d", __LINE__);
            pros::lcd::print(6, "stdin error");
    pros::lcd::print(7, "ln: %d", __LINE__);
            pros::delay(1);
    pros::lcd::print(7, "ln: %d", __LINE__);
        }
        
    pros::lcd::print(7, "ln: %d", __LINE__);
        if(c == 0){
    pros::lcd::print(7, "ln: %d", __LINE__);
            //control operation
            receiveControl();
    pros::lcd::print(7, "ln: %d", __LINE__);
        }
        else
        {
    pros::lcd::print(7, "ln: %d", __LINE__);
            memset(body, 0, STREAM_BUFFER_SZ + 1);
    pros::lcd::print(7, "ln: %d", __LINE__);
            fread((char*)body, 1, c, stdin);
    pros::lcd::print(7, "ln: %d", __LINE__);
#ifdef VEX_SERIAL_VERBOSE
            pros::lcd::print(6, "%02X %02X %02X %02X %02X %02X %02X %02X", body[0], body[1], body[2], body[3], body[4], body[5], body[6], body[7]);
            pros::delay(500);
#endif
    pros::lcd::print(7, "ln: %d", __LINE__);
            if(callback != NULL){
    pros::lcd::print(7, "ln: %d", __LINE__);
                callback(body, c);
    pros::lcd::print(7, "ln: %d", __LINE__);
            }
    pros::lcd::print(7, "ln: %d", __LINE__);
        }
    pros::lcd::print(7, "ln: %d", __LINE__);
    }
}

void VexSerial::receiveControl(){
    pros::lcd::print(7, "ln: %d", __LINE__);
    uint8_t c = fgetc(stdin);
    pros::lcd::print(7, "ln: %d", __LINE__);
    switch (c)
    {
    case HELLO_MSG:
    pros::lcd::print(7, "ln: %d", __LINE__);
        //received hello
        sendHelloAck();
    pros::lcd::print(7, "ln: %d", __LINE__);
        clientConnected=true;
    pros::lcd::print(7, "ln: %d", __LINE__);
        break;
    case HELLO_ACK_MSG:
    pros::lcd::print(7, "ln: %d", __LINE__);
        //received hello-ack
        clientConnected=true;
    pros::lcd::print(7, "ln: %d", __LINE__);
        break;
    case GOODBYE_MSG:
    pros::lcd::print(7, "ln: %d", __LINE__);
        //received goodbye
        sendGoodbyeAck();
    pros::lcd::print(7, "ln: %d", __LINE__);
        clientConnected=false;
    pros::lcd::print(7, "ln: %d", __LINE__);
        break;
    case GOODBYE_ACK_MSG:
    pros::lcd::print(7, "ln: %d", __LINE__);
        //received goodbye ack
        clientConnected=false;
    pros::lcd::print(7, "ln: %d", __LINE__);
        break;
    case ECHO_SIG:
    pros::lcd::print(7, "ln: %d", __LINE__);
        sendEchoAck();
    pros::lcd::print(7, "ln: %d", __LINE__);
        break;
    case SYNC_MSG:
    pros::lcd::print(7, "ln: %d", __LINE__);
        sendSyncAck();
    pros::lcd::print(7, "ln: %d", __LINE__);
        break;
    default:
    pros::lcd::print(7, "ln: %d", __LINE__);
        break;
    }
    pros::lcd::print(7, "ln: %d", __LINE__);
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
#ifdef VEX_SERIAL_VERBOSE
        pros::lcd::print(4, "%02X %02X %02X %02X %02X %02X %02X %02X", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
#endif
        fwrite(buffer, 1, thisSize+1, stdout);
        fflush(stdout);
        offset += thisSize;
    }
}

void VexSerial::tryConnect(const int min_s_retry, const bool block){
    pros::lcd::print(7, "Attempting connection...");
    if(clientConnected == false){
        time_t nowTime = time(NULL);
        if(nowTime - min_s_retry < last_connect_attempt_time){
            if(block){
                pros::delay((nowTime - min_s_retry)*1000);
            }
            else{
                return;
            }

            if(clientConnected){
                return;
            }
        }

        sendHello();
        last_connect_attempt_time = time(NULL);
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