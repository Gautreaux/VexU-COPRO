
#include "main.h"
#include "util.h"

// in degrees
#define PID_THRESHOLD 1

#define PID_P_CONST 1
#define PID_I_CONST .2
#define PID_D_CONST 4

extern pros::Imu IMU;

namespace SpencerPID
{
bool isPIDRunning(void);

void stopPID(void);

// blocking wait for PID to finish
void waitPIDFinish(void);

// positive is CCW
void rotateDegrees(const double degrees);

// called to update the PID loop
void updatePID(void);
} // namespace SpencerPID

