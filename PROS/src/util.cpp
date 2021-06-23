#include "util.h"

Motorgroup leftDrive;
Motorgroup rightDrive;
Motorgroup intake;
Motorgroup rollers;
Motorgroup topRollers;

void updateMotorGroup(const Motorgroup& mg, const int32_t voltage){
    for(auto& motor : mg){
#ifdef LOW_POWER_MODE
        motor.move(voltage/8);
#else   
        motor.move(voltage);
#endif
    }
}

void arcadeDrive(int32_t x, int32_t y, const bool force){
    if(std::abs(x) < CONTROLLER_THRESHOLD){
        x = 0;
    }

    if(std::abs(y) < CONTROLLER_THRESHOLD){
        y = 0;
    }

    if(!force && x == 0 && y == 0){
        return;
    }

    // if(y == 0){
    //     updateDrive(x, -x);
    //     return;
    // }else if(x == 0){
    //     updateDrive(y, y);
    //     return;
    // }

    int32_t tank_left = y + x;
    int32_t tank_right = y + -x;
    updateDrive(tank_left, tank_right);

    // int32_t m = std::max(std::abs(tank_left), std::abs(tank_right));

    // updateDrive(tank_left / m, tank_right / m);
}

void tankDrive(int32_t x, int32_t y, const bool force){
    if(std::abs(x) < CONTROLLER_THRESHOLD){
        x = 0;
    }

    if(std::abs(y) < CONTROLLER_THRESHOLD){
        y = 0;
    }

    if(!force && x == 0 && y == 0){
        return;
    }

    updateDrive(x,y);
}

void stopAll(void){
    updateMotorGroup(leftDrive, 0);
    updateMotorGroup(rightDrive, 0);
    updateMotorGroup(intake, 0);
    updateMotorGroup(rollers, 0);
}
