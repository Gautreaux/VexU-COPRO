#include <stdio.h> // for printf
#include <string.h> // for memset, memcpy
#include <stdint.h>

#include "../PROS/include/vexMessenger.h"

// #define SERIAL_FILE_PATH "/dev/ttyACM1"

int main(int argc, char** argv){

    uint8_t buffer[128];
    uint8_t len;

    VexMessenger::v_messenger->connect();

    printf("Meessenger Connected\n");

    while(true){

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        // if(VexMessenger::v_messenger->isConnected()){
        //     len = 0;
        //     memset(buffer, 0, 128);
        //     if(VexMessenger::v_messenger->readDataMessage(buffer, len, 1000)){
        //         printf("New Message: (%d) %02X %02X %02X %02X %02X %02X %02X %02X : %s\n",
        //             len, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], (char *)(buffer));
        //     }else{
        //         printf("No Message\n");
        //     }
        // }else{
        //     if(VexMessenger::v_messenger->tryConnect(2000)){
        //         printf("Connection Succeded\n");
        //     }else{
        //         printf("Connection failed\n");
        //     }
        // }

    }
}