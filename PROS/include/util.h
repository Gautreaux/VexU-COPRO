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
void updateDrive(int32_t leftY, int32_t rightY);

// convert arcade inputs into tank inputs
void arcadeDrive(int32_t x, int32_t y);

void stopAll(void);


#endif