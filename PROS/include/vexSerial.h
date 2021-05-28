#include "main.h"
#include <time.h>

#define STREAM_BUFFER_SZ 256

//#define VEX_SERIAL_VERBOSE

#define HELLO_MSG '\x00'
#define HELLO_ACK_MSG '\x01'
#define GOODBYE_MSG '\x09'
#define GOODBYE_ACK_MSG '\x0A'
#define ECHO_SIG '\x02'
#define ECHO_ACK_SIG '\x03'
#define SYNC_MSG '\x04'
#define SYNC_ACK_MSG '\x05'

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

    // void sendEcho(const uint8_t* const buffer, const uint8_t size);
    void sendEchoAck(void);

    // void sendSync(const uint8_t data);
    void sendSyncAck(void);

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