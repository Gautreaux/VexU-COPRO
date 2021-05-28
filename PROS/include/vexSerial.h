#include "main.h"

#define STREAM_BUFFER_SZ 256

class VexSerial;

using CallbackFunctionType = void (*) (const uint8_t * const, const uint8_t);

class VexSerial {
private:
    // bool isOk;
    bool taskOk;
    pros::Task receiveTask;
    
    char strBuff[STREAM_BUFFER_SZ];

    CallbackFunctionType callback;
    //TODO - how to start a task...

    void sendHello(void);
    void sendHelloAck(void);
    void sendGoodbye(void);
    void sendGoodbyeAck(void);

    void receiveControl(void);

    VexSerial(void);
    ~VexSerial();
public:
    static VexSerial* const v_ser;
    void sendData(const uint8_t* const buff, const size_t size);
    void receiveData(void);

    inline void setCallback(CallbackFunctionType c){
        callback = c;
    }
};

void receiveDataWrapper(void);