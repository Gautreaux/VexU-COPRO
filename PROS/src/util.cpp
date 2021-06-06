#include "util.h"

Motorgroup leftDrive;
Motorgroup rightDrive;
Motorgroup intake;
Motorgroup rollers;

void updateMotorGroup(const Motorgroup& mg, const int32_t voltage){
    for(auto& motor : mg){
#ifdef LOW_POWER_MODE
        motor.move(voltage/8);
#else   
        motor.move(voltage);
#endif
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

void stopAll(void){
    updateMotorGroup(leftDrive, 0);
    updateMotorGroup(rightDrive, 0);
    updateMotorGroup(intake, 0);
    updateMotorGroup(rollers, 0);
}