  
#include <iostream>
#include <sys/types.h> // for open
#include <sys/stat.h> // for open
#include <fcntl.h> // for open
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#define SERIAL_FILE_PATH "/dev/ttyACM1"

#include "vexSerial.h"

int
set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        if (tcgetattr (fd, &tty) != 0)
        {
                printf("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void
set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0){
                printf("error %d setting term attributes", errno);
        }
}

int main(){
    int serialFD = open(SERIAL_FILE_PATH, O_RDWR | O_NOCTTY | O_SYNC);
    if(serialFD < 0){
        printf("Error %d occurred whic opening serial file.\n", serialFD);
    }
    // make the file non-blocking
    // fcntl(serialFD, F_SETFL, O_NONBLOCK);

    set_interface_attribs(serialFD, B115200, 0);
    set_blocking(serialFD, 1);

    char helloAck[] = {05, 04, 00, 00, 02, 00};
    char hello[]    = {05, 04, 00, 00, 01, 00};

    // write(serialFD, hello, 6);

    VexSerial::SerialFD = serialFD;
    int ctr = 0;

    printf("WE MADE IT HERE\n");
    while(true){
        // char c;
        // ssize_t sz = read(serialFD, &c, 1);
        // if(sz > 0){
        //     printf("Bytes %d %02X\n", sz, c);
        // }
        // else{
        //     printf("GAH\n");
        // }

        if(++ctr == 3){
            write(serialFD, hello, 6);
        }

        uint8_t msgBuff[128];
        memset(msgBuff, 0, 128);
        uint8_t size = 0;

        VexSerial::receiveMessage(msgBuff, size);

        printf("Received a message of size %d : ", size);
        for(unsigned int i = 0; i < size; i++){
            printf("%02X ", msgBuff[i]);
        }
        printf("\n");
        
    }
}