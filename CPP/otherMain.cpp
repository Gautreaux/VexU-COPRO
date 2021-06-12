#include <iostream>
#include <sys/types.h> // for open
#include <sys/stat.h> // for open
#include <fcntl.h> // for open
#include <unistd.h>
#define SERIAL_FILE_PATH "/dev/ttyACM1"

int main(){
    // FILE* fpointer;
    // fpointer = fopen(SERIAL_FILE_PATH, "r");
    // printf("WE MADE IT HERE\n");
    // while(true){
    //     char c;
    //     fgets(&c, 1, fpointer);
    //     printf("Bytes %02X\n", c);
    // }

    int serialFD = open(SERIAL_FILE_PATH, O_RDONLY);
    if(serialFD < 0){
        printf("Error %d occurred whic opening serial file.\n", serialFD);
    }
    // make the file non-blocking
    // fcntl(serialFD, F_SETFL, O_NONBLOCK);

    printf("WE MADE IT HERE\n");
    while(true){
        char c;
        ssize_t sz = read(serialFD, &c, 1);
        if(sz > 0){
            printf("Bytes %d %02X\n", sz, c);
        }
        // else if(c == '\x0A'){
        //     printf("0A: %d\n", sz);
        // }
    }
}