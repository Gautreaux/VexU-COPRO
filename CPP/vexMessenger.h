#ifndef __VEX_MESSENGER__
#define __VEX_MESSENGER__

#include <stdexcept>

#include "vexSerial.h"

#define TIMEOUT_MAX (-1u)

//for tryConnect/tryDisconnect, how frequently to retry
#define RETRY_INTERVAL_MS 200u

namespace VexMessenger {
    class UnexpectedDisconnection : public std::exception
    {
    };

    // start up the messenger
    //  must call once
    //  should probably call only once
    void initMessenger(void);

    // return true iff connected
    bool isConnected(void);

    // send buffer as a data message
    void sendMessage(uint8_t const * const buff, uint8_t len);

    // try and connect,
    //   return true if connected before timeout
    //      else return false
    bool tryConnect(uint32_t const timeout_ms = 1000);

    // blocking until connected
    inline void connect(void)
    {
        tryConnect(TIMEOUT_MAX);
    }

    // read messages until a data message
    //  returns true if a message was successfully read
    //  returns false if a disconnect occurs before timeout
    //  returns false if a timeout occurs before the next data message
    //  immediately returns false if disconnected
    bool readDataMessage(uint8_t * const buff, uint8_t& len, uint32_t const timeout_ms);

    // read messages until a data message
    //  will throw UnexpectedDisconnection if disconnected data message
    //    or if called while disconnected
    void readDataMessageBlocking(uint8_t * const buff, uint8_t& len);
};

#endif //__VEX_MESSENGER__