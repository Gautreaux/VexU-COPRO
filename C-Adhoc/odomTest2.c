#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

volatile int running = 1;

void sigInt(int s){
    printf("Signal Received\n");
    running = 0;
}

int main(){
    // printf("HELLO\n");

    const char* LEFT_MOUSE_PATH = "/dev/input/mouse0";
    // const char* RIGHT_MOUSE_PATH = "/dev/input/mouse2";

    int leftMouse = open(LEFT_MOUSE_PATH, O_RDONLY);
    // int rightMouse = open(RIGHT_MOUSE_PATH, O_RDONLY);

    if(leftMouse == -1){
        printf("Error opening leftMouse\n");
    }
    // if(rightMouse == -1){
    //     printf("Error opening rightMouse\n");
    // }
    printf("Mice opened successfully\n");

    signal(SIGINT, sigInt);


    // fd_set rfs;
    // int fd_max = (leftMouse > rightMouse) ? (leftMouse) : (rightMouse);

    int64_t times[] = {0,0,0,0};
    int64_t deltas[] = {0,0,0,0};

    char buffer[3];

    // struct timeval timeout;
    // int j;

    while(running){
        // memset(buffer, 0, 3);
        // memset(&timeout, 0, sizeof(timeout));
        // timeout.tv_sec = 1;
        // FD_ZERO(&rfs);
        // FD_SET(leftMouse, &rfs);
        // FD_SET(rightMouse, &rfs);
        // if((j = select(fd_max+1, &rfs, NULL, NULL, &timeout)) == 0){
        //     // printf("Timeout\n");
        // };

        // if(j < 0){
        //     break;
        // }

        // if(FD_ISSET(leftMouse, &rfs)){
            // printf("Left mouse\n");
            read(leftMouse, buffer, 3);

            if(!running){
                printf("Early break\n");
                break;
            }

            // printf("%02X %02X %02X %d\n", buffer[0], buffer[1], buffer[2], 0+(int8_t)(buffer[1]));

            deltas[0] += (int8_t)buffer[1];
            deltas[1] += (int8_t)buffer[2];

            times[0] += (buffer[1] > 0 ? (1) : ((buffer[1] < 0) ? (-1) : (0)));
            times[1] += (buffer[2] > 0 ? (1) : ((buffer[1] < 0) ? (-1) : (0)));
        // }
        // if(FD_ISSET(rightMouse, &rfs)){
        //     // printf("Right mouse\n");
        //     read(rightMouse, buffer, 3);

        //     deltas[2] += (int8_t)buffer[1];
        //     deltas[3] += (int8_t)buffer[2];

        //     times[2] += (buffer[1] > 0 ? (1) : ((buffer[1] < 0) ? (-1) : (0)));
        //     times[3] += (buffer[2] > 0 ? (1) : ((buffer[1] < 0) ? (-1) : (0)));
        // }
    }

    printf("Times: %lld %lld %lld %lld\n", times[0], times[1], times[2], times[3]);
    printf("Deltas: %lld %lld %lld %lld\n", deltas[0], deltas[1], deltas[2], deltas[3]);
}