#ifndef __VEX_CONTROLLER__
#define __VEX_CONTROLLER__

#include "main.h"
#include "util.h"
#include "vexMessenger.h"

enum class COMMAND_ENUM {
    STOP = 0,
    TEXT = 1,
    JITTER = 10,
    START_ROTATION_LEFT = 11,
    START_ROTATION_RIGHT = 12,
    START_SLOW_ROTATION_LEFT = 13,
    START_SLOW_ROTATION_RIGHT = 14,
    START_MAX_ROTATION_LEFT = 17,
    START_MAX_ROTATION_RIGHT = 18,
    READ_IMU = 32,
    RESET_IMU = 33,
    GOAL_POS = 64,
};

extern pros::Imu IMU;

// how fast should start/stop rotation run
//  something in range 0-127
//      but you know, dont do 0, it wont do anything
#ifndef LOW_POWER_MODE
#define ROTATION_INTENSITY 30
#else
#define ROTATION_INTENSITY 127
#endif

#define SLOW_ROTATION_INTENSITY 30
#define MAX_ROTATION_INTENSITY 127

// process a single message instance
void processMessage(uint8_t const  * const buffer, const uint8_t len);

// stop all robot motion
void stop(uint8_t const  * const buffer, const uint8_t len);

// display text to the screen
void text(uint8_t const  * const buffer, const uint8_t len);

void jitter(uint8_t const  * const buffer, const uint8_t len);

void startRotation(uint8_t const  * const buffer, const uint8_t len, const bool isRotationLeft = true, const uint8_t intensity = ROTATION_INTENSITY);

void readIMU(uint8_t const  * const buffer, const uint8_t len);

void resetIMU(uint8_t const  * const buffer, const uint8_t len);

void goalPos(uint8_t const * const buffer, const uint8_t len);

#endif