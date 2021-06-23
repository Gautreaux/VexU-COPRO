
#include "main.h"
#include "util.h"

// in degrees
#define PID_THRESHOLD 1

#define I_MAX 50.0

#define PID_P_CONST 5
#define PID_I_CONST .5
#define PID_D_CONST 3

extern pros::Imu IMU;

namespace SpencerPID
{
bool isPIDRunning(void);

void stopPID(void);

// blocking wait for PID to finish
void waitPIDFinish(void);

// positive is CCW
void rotateDegrees(const double degrees);

void driveStraightInches(const double inches);

// called to update the PID loop
void updatePID(void);
} // namespace SpencerPID

