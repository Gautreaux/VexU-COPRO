#include "vexController.h"



void processMessage(uint8_t const  * const buffer, const uint8_t len){
    switch (COMMAND_ENUM(buffer[0]))
    {
    case COMMAND_ENUM::STOP:
        stop(buffer, len);
        return;
    case COMMAND_ENUM::TEXT:
        text(buffer, len);
        return;
    case COMMAND_ENUM::JITTER:
        jitter(buffer, len);
        return;
    case COMMAND_ENUM::START_ROTATION_LEFT:
        startRotation(buffer, len, true);
        return;
    case COMMAND_ENUM::START_ROTATION_RIGHT:
        startRotation(buffer, len, false);
        return;
    case COMMAND_ENUM::START_SLOW_ROTATION_LEFT:
        startRotation(buffer, len, true, SLOW_ROTATION_INTENSITY);
        return;
    case COMMAND_ENUM::START_SLOW_ROTATION_RIGHT:
        startRotation(buffer, len, false, SLOW_ROTATION_INTENSITY);
        return;
    case COMMAND_ENUM::START_MAX_ROTATION_LEFT:
        startRotation(buffer, len, true, MAX_ROTATION_INTENSITY);
        return;
    case COMMAND_ENUM::START_MAX_ROTATION_RIGHT:
        startRotation(buffer, len, false, MAX_ROTATION_INTENSITY);
        return;
    case COMMAND_ENUM::READ_IMU:
        readIMU(buffer, len);
        return;
    case COMMAND_ENUM::RESET_IMU:
        resetIMU(buffer, len);
        return;
    case COMMAND_ENUM::GOAL_POS:
        goalPos(buffer, len);
        return;
    default:
        pros::lcd::print(LCD_OPEN_7, "Illegal Command: 0x%X (%d)", buffer[0], buffer[0]);
        return;
    }
}

void stop(uint8_t const  * const buffer, const uint8_t len){
    stopAll();
}

void text(uint8_t const  * const buffer, const uint8_t len){
    pros::lcd::print(LCD_OPEN_7, "%s", buffer+1);
}

void jitter(uint8_t const  * const buffer, const uint8_t len){
    //TODO - will need a coro?
}

void startRotation(uint8_t const  * const buffer, const uint8_t len, const bool isRotationLeft, const uint8_t intensity) {
    if(isRotationLeft){
        updateMotorGroup(leftDrive, -intensity);
        updateMotorGroup(rightDrive, intensity);
    }else{
        updateMotorGroup(leftDrive, intensity);
        updateMotorGroup(rightDrive, -intensity);
    }
}

void readIMU(uint8_t const  * const buffer, const uint8_t len){
    char c[32];
    // TODO - if rotation gets large, will overflow c buffer and this 
    //  will return a negative number
    uint8_t sz = snprintf(c, 32, "%.4f", -IMU.get_rotation());
    VexMessenger::v_messenger->sendMessage((uint8_t * const)c, sz);
}

void resetIMU(uint8_t const  * const buffer, const uint8_t len){
    IMU.reset();
}

void goalPos(uint8_t const * const buffer, const uint8_t len){
    int32_t processed = sscanf((const char*)(buffer+1), "%d %d %d %d", goalConst, goalConst + 1, goalConst + 2, goalConst + 3);
    pros::lcd::print(LCD_OPEN_6, "VALS: %d %d %d %d", goalConst[0], goalConst[1], goalConst[2], goalConst[3]);
    return;
}