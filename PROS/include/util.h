#ifndef _UTIL_H_
#define _UTIL_H_

#include "main.h"
#include "config.h"
#include <vector>

using Motorgroup = std::vector<pros::Motor>;

extern Motorgroup leftDrive;
extern Motorgroup rightDrive;
extern Motorgroup intake;
extern Motorgroup rollers;
extern Motorgroup topRollers;

//set all motors in the motor group to the specified voltage
void updateMotorGroup(const Motorgroup& mg, const int32_t voltage);

// basically just a tank drive wrapper
//  does no input checking
//  leftY and rightY should be in range -128, 127
inline void updateDrive(int32_t leftY, int32_t rightY){
    updateMotorGroup(leftDrive, leftY);
    updateMotorGroup(rightDrive, rightY);
}

// arcade drive
//  force - alters how near-zero inputs are handled
//      true - near zero is set to zero and motors are updated
//      false - motors are not updated
void arcadeDrive(int32_t x, int32_t y, const bool force);

// tank drive
//  force - alters how near-zero inputs are handled
//      true - near zero is set to zero and motors are updated
//      false - motors are not updated
void tankDrive(int32_t x, int32_t y, const bool force);

void stopAll(void);


#endif