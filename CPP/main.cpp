#include <stdio.h> // for printf
#include <sys/types.h> // for open
#include <sys/stat.h> // for open
#include <fcntl.h> // for open
#include <unistd.h> // for read
#include <string.h> // for memset, memcpy

#include "../PROS/include/vexSerial.h"

#define SERIAL_FILE_PATH "/dev/ttyACM1"

int main(int argc, char** argv){


    int serialFD = open(SERIAL_FILE_PATH, O_RDONLY);

    if(serialFD == -1){
        printf("Some Error occurred opening the serial FD");
        return 1;
    }

    char buffer[256];

    fcntl(serialFD, F_SETFL, O_NONBLOCK);

    while(1){
        memset(buffer, 0, 256);
        int bytes_read = read(serialFD, buffer, 256);

        if(bytes_read <= 0){
            printf("%d No data\n", bytes_read);
            sleep(1);
            continue;
        }

        printf("(%d) : %s\n", bytes_read, buffer);
        if(bytes_read == 2){
            printf("  %02X %02X\n", buffer[0], buffer[1]);
        }
    }
}