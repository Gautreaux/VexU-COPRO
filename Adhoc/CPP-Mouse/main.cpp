#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <math.h>
#include <atomic>
#include <thread>

std::atomic_bool running{true};
std::atomic_uint64_t deltaBuffer{0};

void sigInt(int s)
{
    printf("Signal Received\n");
    running.store(false);
}


void threadTarget()
{
    const char *LEFT_MOUSE_PATH = "/dev/input/mouse0";
    const char *RIGHT_MOUSE_PATH = "/dev/input/mouse2";

    int leftMouse = open(LEFT_MOUSE_PATH, O_RDONLY);
    int rightMouse = open(RIGHT_MOUSE_PATH, O_RDONLY);

    if (leftMouse == -1)
    {
        printf("Error opening leftMouse\n");
    }
    if (rightMouse == -1)
    {
        printf("Error opening rightMouse\n");
    }
    printf("Mice opened successfully\n");

    signal(SIGINT, sigInt);

    fd_set rfs;
    int fd_max = (leftMouse > rightMouse) ? (leftMouse) : (rightMouse);

    // int64_t times[] = {0, 0, 0, 0};

    char buffer[3];

    struct timeval timeout;
    int j;

    // int8_t bigB = 0;

    while (running.load())
    {
        int16_t deltas[] = {0, 0, 0, 0};

        memset(buffer, 0, 3);
        memset(&timeout, 0, sizeof(timeout));
        timeout.tv_sec = 1;
        FD_ZERO(&rfs);
        FD_SET(leftMouse, &rfs);
        FD_SET(rightMouse, &rfs);
        if ((j = select(fd_max + 1, &rfs, NULL, NULL, &timeout)) == 0)
        {
            // printf("Timeout\n");
        };

        if (j < 0)
        {
            break;
        }

        if (FD_ISSET(leftMouse, &rfs))
        {
            // printf("Left mouse\n");
            read(leftMouse, buffer, 3);

            // printf("%02X %02X %02X %d\n", buffer[0], buffer[1], buffer[2], 0+(int8_t)(buffer[1]));

            deltas[0] = (int8_t)buffer[1];
            deltas[1] = (int8_t)buffer[2];

            // times[0] += (buffer[1] > 0 ? (1) : ((buffer[1] < 0) ? (-1) : (0)));
            // times[1] += (buffer[2] > 0 ? (1) : ((buffer[1] < 0) ? (-1) : (0)));

            // bigB = std::max(bigB, (int8_t)std::abs(buffer[1]));
            // bigB = std::max(bigB, (int8_t)std::abs(buffer[2]));
        }
        if (FD_ISSET(rightMouse, &rfs))
        {
            // printf("Right mouse\n");
            read(rightMouse, buffer, 3);

            deltas[2] = (int8_t)buffer[1];
            deltas[3] = (int8_t)buffer[2];

            // times[2] += (buffer[1] > 0 ? (1) : ((buffer[1] < 0) ? (-1) : (0)));
            // times[3] += (buffer[2] > 0 ? (1) : ((buffer[1] < 0) ? (-1) : (0)));

            // bigB = std::max(bigB, (int8_t)std::abs(buffer[1]));
            // bigB = std::max(bigB, (int8_t)std::abs(buffer[2]));
        }

        // printf("Times: %lld %lld %lld %lld\n", times[0], times[1], times[2], times[3]);
        // printf("Deltas: %lld %lld %lld %lld\n", deltas[0], deltas[1], deltas[2], deltas[3]);
        // printf("BigB: %d\n", bigB);

        uint64_t newValue, value = (deltaBuffer.load(std::memory_order::memory_order_acquire));
        do{
            // printf("value: %lld\n", value);
            newValue = value;
            auto valuePtr = (int16_t*)(&newValue);
            valuePtr[0] += deltas[0];
            valuePtr[1] += deltas[1];
            valuePtr[2] += deltas[2];
            valuePtr[3] += deltas[3];
        }while(!deltaBuffer.compare_exchange_weak(value, newValue, std::memory_order::memory_order_acq_rel));
    }
}

int main()
{
    std::thread myThread(threadTarget);

    int largestXDiff = 0;

    int64_t totals[] = {0, 0, 0, 0};

    constexpr double distanceBetweenMice = 1500;

    double lastRotation = 0.0;
    double absolute100percentCorrectLocation[2] = {0,0};

    while(running.load()){
        sleep(1);

        uint64_t deltas_64 = deltaBuffer.load();
        int16_t* deltasPtr = (int16_t*)(&deltas_64);
        uint_fast8_t ctr = 0;
        while(running.load()){
            if(ctr++ > 100){
                deltas_64 = deltaBuffer.exchange(0, std::memory_order::memory_order_acq_rel);
                printf("Read Loop exiting early error is %d.\n", abs(deltasPtr[0] - deltasPtr[2]));
                break;
            }
            // deltas_64 = deltaBuffer.exchange(0, std::memory_order::memory_order_acq_rel);
            if(abs(deltasPtr[0] - deltasPtr[2]) > 256){
                deltas_64 = deltaBuffer.load();
                continue;
            }
            if(deltaBuffer.compare_exchange_weak(deltas_64, 0, std::memory_order::memory_order_acq_rel)){
                break;
            }
        }

        printf("Deltas: % 6d % 6d % 6d % 6d\n", deltasPtr[0], deltasPtr[1], deltasPtr[2], deltasPtr[3]);
        printf("  Totals: % 6lld % 6lld % 6lld % 6lld\n", totals[0], totals[1], totals[2], totals[3]);

        largestXDiff = std::max(largestXDiff, abs(deltasPtr[0] - deltasPtr[2]));

        printf("  Largest X: % 6d\n", largestXDiff);

        totals[0] += deltasPtr[0];
        totals[1] += deltasPtr[1];
        totals[2] += deltasPtr[2];
        totals[3] += deltasPtr[3];

        printf("  Cumulative x error: %lld\n", abs(totals[0] - totals[2]));

        double deltaX = (deltasPtr[0] + deltasPtr[1]) / 2;
        double deltaY_L = deltasPtr[1];
        double deltaY_R = deltasPtr[3];

        //TODO - scaling constant
        double yTotal_L = totals[1];
        double yTotal_R = totals[3];

        auto nowRotation = (yTotal_L - yTotal_R) / distanceBetweenMice;

        auto deltaRotation = nowRotation - lastRotation;

        double localOffset[2];
        if(deltaRotation == 0){
            localOffset[0] = deltaX;
            localOffset[1] = deltaY_R;
        }else{
            auto p = 2 * sin(deltaRotation / 2);
            localOffset[0] = p * (deltaX / deltaRotation);
            localOffset[1] = p * (deltaY_R / deltaRotation + (distanceBetweenMice / 2));
        }

        double averageRotation = (lastRotation + nowRotation) / 2;

        double polar[2] = {
            sqrt(localOffset[0] * localOffset[0] + localOffset[1] * localOffset[1]),
            atan2(localOffset[1], localOffset[0])
        };

        polar[2] -= averageRotation;

        double actualOffset[2] = {
            polar[0] * cos(polar[1]),
            polar[0] * sin(polar[1])
        };

        absolute100percentCorrectLocation[0] += actualOffset[0];
        absolute100percentCorrectLocation[1] += actualOffset[1];
        lastRotation = nowRotation;

        printf("% 12.4f % 12.4f % 12.4f\n", absolute100percentCorrectLocation[0], absolute100percentCorrectLocation[1], lastRotation);
    }

    myThread.join();

    // printf("HELLO\n");

}