#include "main.h"
#include <time.h>

#define STREAM_BUFFER_SZ 256

class VexSerial;

using CallbackFunctionType = void (*) (const uint8_t * const, const uint8_t);

class VexSerial {
private:
    bool taskOk;
    bool clientConnected;
    CallbackFunctionType callback;
    pros::Task receiveTask;

    time_t last_connect_attempt_time;

    char strBuff[STREAM_BUFFER_SZ];

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
    
    void tryConnect(const int min_s_retry = 1, const bool block = false);
    void disconnect(void);

    inline bool isConnected(void) { return clientConnected; }
};

void receiveDataWrapper(void* params);