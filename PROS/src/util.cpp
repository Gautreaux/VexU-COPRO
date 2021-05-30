#include "util.h"

Motorgroup leftDrive;
Motorgroup rightDrive;
Motorgroup intake;
Motorgroup rollers;

void updateMotorGroup(const Motorgroup& mg, const int32_t voltage){
    for(auto& motor : mg){
        motor.move(voltage);
    }
}

void updateDrive(int32_t leftY, int32_t rightY){
    if(std::abs(leftY) < CONTROLLER_THRESHOLD){
        leftY = 0;
    }

    if(std::abs(rightY) < CONTROLLER_THRESHOLD){
        rightY = 0;
    }

    updateMotorGroup(leftDrive, leftY);
    updateMotorGroup(rightDrive, rightY);
}