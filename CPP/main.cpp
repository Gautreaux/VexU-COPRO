//TODO - some of these can be removed
#include <iostream>
#include <sys/types.h> // for open
#include <sys/stat.h> // for open
#include <fcntl.h> // for open
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

#include "vexSerial.h"
#include "vexMessenger.h"
#include "utils.h"


int main(){


    // write(serialFD, hello, 6);
    VexSerial::SerialFD = utils::getSetupSerialFD();

    if(VexSerial::SerialFD < 0){
        printf("Fatal error opening serial port.\n");
        exit(-1);
    }

    VexMessenger::initMessenger();
    VexMessenger::connect();

    char textMessage[] = " myText";
    textMessage[0] = 1;
    VexMessenger::sendMessage((const uint8_t *)textMessage, 8);

    printf("WE MADE IT HERE\n");
    while(true){
        // // char c;
        // // ssize_t sz = read(serialFD, &c, 1);
        // // if(sz > 0){
        // //     printf("Bytes %d %02X\n", sz, c);
        // // }
        // // else{
        // //     printf("GAH\n");
        // // }
        uint8_t msgBuff[128];
        memset(msgBuff, 0, 128);
        uint8_t size = 0;

        // // VexSerial::receiveMessage(msgBuff, size);

        // printf("Received a message of size %d : ", size);
        // for(unsigned int i = 0; i < size; i++){
        //     printf("%02X ", msgBuff[i]);
        // }
        // printf("\n");

        if(VexMessenger::readDataMessage(msgBuff, size, 2000)){
                printf("New message (%d): %s\n", size, msgBuff);
        }else{
                printf("No message received.\n");
        }

        // std::string s;
        // printf("Enter message:\n");
        // std::cin >> s;
        // //TODO - do something with this
    }
}