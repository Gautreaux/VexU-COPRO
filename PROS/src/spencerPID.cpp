#include "spencerPID.h"

namespace SpencerPID{
    namespace {
        double targetRotation = 0;
        bool pidRunning = false;
        uint32_t pidStartTime = 0;
        uint32_t pidLastUpdateTime = 0;
        double totalError = 0;
        double lastInstantError = 0;

        double maxI = 0;

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
        pidStartTime = pros::millis() / 1000.0;
        pidLastUpdateTime = pidStartTime;
        maxI = 0;
    }

    void driveStraightInches(const double inches){
        //TODO
    }

    void updatePID(void){
        if(!pidRunning){
            return;
        }

        auto nowError = (-IMU.get_rotation()) - targetRotation;

        pros::lcd::print(LCD_OPEN_6, "%f", nowError);

        if(abs(nowError) < PID_THRESHOLD){
            if(consecLowCtr > 2){
                stopDrive();
                pidRunning = false;
                return;
            }else{
                consecLowCtr++;
            }
        }else{
            consecLowCtr = 0;
        }

        double nowTime = pros::millis() / 1000.0;
        double elapsedTime = nowTime - pidLastUpdateTime;
        pidLastUpdateTime = nowTime;

        totalError += nowError * PID_I_CONST * elapsedTime;

        totalError = std::max(std::min(totalError, I_MAX), -I_MAX);

        double errorThingy = 0;
        if (elapsedTime != 0)
        {
            errorThingy = (nowError - lastInstantError) / elapsedTime;
        }

        lastInstantError = nowError;

        maxI = std::max(std::abs(totalError), maxI);
        pros::lcd::print(LCD_OPEN_7, "PID: %.03f %.03f %.03f %.03f", nowError * PID_P_CONST, totalError, errorThingy * PID_D_CONST, maxI);

        int k = (
              (nowError * PID_P_CONST) 
            + (totalError) 
            + (errorThingy * PID_D_CONST)
        );

        updateMotorGroup(leftDrive, k);
        updateMotorGroup(rightDrive, -k);
    }
}
