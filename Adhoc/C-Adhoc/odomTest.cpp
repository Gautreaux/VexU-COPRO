// #include <iostream>
#include <string.h>
// #include <fstream>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>

#include <stropts.h>
#include <poll.h>


using namespace std;

volatile bool running = true;

void sigInt(int s)
{
    printf("SIG CAUGHT\n");
    running = false;
}

int main(){
    const char* LEFT_MOUSE_PATH = "/dev/input/mouse0";
    const char* RIGHT_MOUSE_PATH = "/dev/input/mouse2";

    int leftMouse;
    int rightMouse;

    leftMouse = open(LEFT_MOUSE_PATH, O_RDONLY);
    rightMouse = open(RIGHT_MOUSE_PATH, O_RDONLY);

    if(leftMouse == NULL){
        printf("Error opening left mouse\n");
        return 1;
    }

    if(rightMouse == NULL){
        printf("Error opening right mouse\n");
        return 1;
    }


    // ifstream leftMouse(LEFT_MOUSE_PATH);

    // if(!leftMouse.is_open()){
    //     cout << "Left mouse failed to open" << endl;
    // }

    // ifstream rightMouse(RIGHT_MOUSE_PATH);
    // if(!rightMouse.is_open()){
    //     cout << "Right mouse failed to open" << endl;
    // }



    signal(SIGINT, sigInt);
    int64_t deltas[] = {0,0,0,0};
    int64_t times[] = {0,0,0,0};
    char buffer[3];

    struct pollfd fds[2];
    fds[0].fd = leftMouse;
    fds[1].fd = rightMouse;

    while(running){
        // memset(buffer, 0, 3);
        // leftMouse.read(buffer, 3);
        // printf("%0X %0X %0X\n", buffer[0], buffer[1], buffer[2]);

        // fgets(buffer, 3, leftMouse);
        // times[0] += 1;
    }

    printf("Times: %lld %lld %lld %lld\n", times[0], times[1], times[2], times[3]);
    printf("Deltas: %lld %lld %lld %lld\n", deltas[0], deltas[1], deltas[2], deltas[3]);
}