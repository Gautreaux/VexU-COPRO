#include "spencerPID.h"

namespace SpencerPID{
    namespace {
        double targetRotation = 0;
        bool pidRunning = false;
        uint32_t pidStartTime = 0;
        double totalError = 0;
        double lastInstantError = 0;

        // tracks qty consecutive low power intsances
        //  eventually stopping stuck loops
        int consecLowCtr = 0;
    }

    bool isPIDRunning(void){
        return pidRunning;
    }

    void stopPID(void){
        pidRunning = false;
    }

    void waitPIDFinish(void){
        while(pidRunning){
            updatePID();
            pros::delay(10);
        }
    }

    void rotateDegrees(const double degrees){
        targetRotation = (-IMU.get_rotation() + degrees);
        

        pidRunning = true;
        totalError = 0;
        pidStartTime = pros::millis();
    }

    void updatePID(void){
        if(!pidRunning){
            return;
        }

        auto nowError = (-IMU.get_rotation()) - targetRotation;

        pros::lcd::print(LCD_OPEN_6, "%f", nowError);

        if(abs(nowError) < PID_THRESHOLD){
            stopDrive();
            pidRunning = false;
            return;
        }

        // auto elapsedTime = pros::millis() - pidStartTime;

        totalError += nowError;

        auto errorThingy = (nowError - lastInstantError);

        lastInstantError = nowError;

        int k = (
              (nowError * PID_P_CONST) 
            + (totalError * PID_I_CONST) 
            + (errorThingy * PID_D_CONST)
        );

        updateMotorGroup(leftDrive, k);
        updateMotorGroup(rightDrive, -k);
    }
}
