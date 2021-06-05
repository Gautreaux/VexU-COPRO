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
    case COMMAND_ENUM::READ_IMU:
        readIMU(buffer, len);
        return;
    case COMMAND_ENUM::RESET_IMU:
        resetIMU(buffer, len);
        return;
    default:
        pros::lcd::print(7, "Illegal Command: 0x%X", buffer[0]);
        return;
    }
}

void stop(uint8_t const  * const buffer, const uint8_t len){
    updateMotorGroup(leftDrive, 0);
    updateMotorGroup(rightDrive, 0);
    updateMotorGroup(intake, 0);
    updateMotorGroup(rollers, 0);
}

void text(uint8_t const  * const buffer, const uint8_t len){
    pros::lcd::print(7, "%s", buffer+1);
}

void jitter(uint8_t const  * const buffer, const uint8_t len){
    //TODO - will need a coro?
}

void startRotation(uint8_t const  * const buffer, const uint8_t len, const bool isRotationLeft) {
    if(isRotationLeft){
        updateMotorGroup(leftDrive, -ROTATION_INTENSITY);
        updateMotorGroup(rightDrive, ROTATION_INTENSITY);
    }else{
        updateMotorGroup(leftDrive, ROTATION_INTENSITY);
        updateMotorGroup(rightDrive, -ROTATION_INTENSITY);
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