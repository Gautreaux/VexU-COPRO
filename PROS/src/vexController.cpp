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
    default:
        pros::lcd::print(7, "Illegal Command: %X", buffer[0]);
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