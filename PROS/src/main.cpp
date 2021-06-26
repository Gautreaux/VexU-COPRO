#include "main.h"

#include <utility>
#include "vexController.h"
// #include "vexSerial.h"
#include "vexMessenger.h"
#include "util.h"
#include "spencerPID.h"
#include "vexBehavior.h"

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
pros::Imu IMU(IMU_PORT);
double last_target = 0;
double last_current = 0;

void run_pid(
        std::function<double()> current_function,
        const std::function<void(double)>& apply_function,
        double target,
        double p,
        double i,
        double d,
        double max_i,
        double threshold,
        uint32_t count_needed
){
    last_target = target;
    last_current = current_function();
    SpencerPID::PID pid{target, std::move(current_function), p, i, d, max_i};
    uint32_t count = 0;
    while (count < count_needed){
        auto out = pid.run();
        if (std::abs(out.error) < threshold){
            count++;
        }
        else{
            count = 0;
        }
        apply_function(out.power);
        pros::delay(5);
    }
    apply_function(0);
}

#ifdef ROBOT_TARGET_15
constexpr double TURN_P = 10;
constexpr double TURN_I = 1;
constexpr double TURN_D = 0.5;
constexpr double TURN_I_MAX = 50;

constexpr double DRIVE_P = 50;
constexpr double DRIVE_I = 0;
constexpr double DRIVE_D = 5;
constexpr double DRIVE_I_MAX = 50;

constexpr double DRIVE_MATCH_P = 2;
constexpr double DRIVE_MATCH_I = 0;
constexpr double DRIVE_MATCH_D = 0;
constexpr double DRIVE_MATCH_MAX_I = 50;
#endif
#ifdef ROBOT_TARGET_24
constexpr double TURN_P = 5;
constexpr double TURN_I = 0.5;
constexpr double TURN_D = 3;
constexpr double TURN_I_MAX = 50;

constexpr double DRIVE_P = 5;
constexpr double DRIVE_I = 0.5;
constexpr double DRIVE_D = 3;
constexpr double DRIVE_I_MAX = 50;

constexpr double DRIVE_MATCH_P = 5;
constexpr double DRIVE_MATCH_I = 0.5;
constexpr double DRIVE_MATCH_D = 3;
constexpr double DRIVE_MATCH_MAX_I = 50;
#endif

void turn(double degrees, Motorgroup& left_drive, Motorgroup& right_drive){
    const auto start = IMU.get_rotation();
    const auto target = start + degrees;
    run_pid(
            [](){return IMU.get_rotation();},
            [left_drive, right_drive](double in_val){
                for(auto& motor : left_drive){
                    motor = -(int32_t)in_val;
                }
                for(auto& motor: right_drive){
                    motor = (int32_t)in_val;
                }
            },
            target,
            TURN_P, TURN_I, TURN_D, TURN_I_MAX, 3, 20);
}

void turn_to_angle(double angle, Motorgroup& left_drive, Motorgroup& right_drive){
    run_pid(
            [](){return IMU.get_heading();},
            [left_drive, right_drive](double in_val){
                for(auto& motor : left_drive){
                    motor = -(int32_t)in_val;
                }
                for(auto& motor: right_drive){
                    motor = (int32_t)in_val;
                }
            },
            angle,
            TURN_P, TURN_I, TURN_D, TURN_I_MAX, 3, 20);
}

void drive_for_distance(int32_t distance, Motorgroup& left_drive, Motorgroup& right_drive){
    const auto start_angle = IMU.get_rotation();
    const auto left_start = left_drive[0].get_raw_position(nullptr);
    const auto right_start = right_drive[0].get_raw_position(nullptr);

    SpencerPID::PID left_distance_pid{
            (double)left_start + distance,
            [&left_drive](){
                return left_drive[0].get_raw_position(nullptr);
            },
            DRIVE_P, DRIVE_I, DRIVE_D, DRIVE_I_MAX
    };
    SpencerPID::PID right_distance_pid{
            (double)right_start + distance,
            [&right_drive](){
                return right_drive[0].get_raw_position(nullptr);
            },
            DRIVE_P, DRIVE_I, DRIVE_D, DRIVE_I_MAX
    };
    SpencerPID::PID match_pid{
            start_angle,
            [](){
                return IMU.get_rotation();
            },
            DRIVE_MATCH_P, DRIVE_MATCH_I, DRIVE_MATCH_D, DRIVE_MATCH_MAX_I
    };

    uint32_t count = 0;

    constexpr double forward_backward_tolerance = 50;

    while(count < 100){
        auto left_result = left_distance_pid.run();
        auto right_result = right_distance_pid.run();
        auto match_result = match_pid.run();

        if(
                left_result.error < forward_backward_tolerance
                && right_result.error < forward_backward_tolerance
                && match_result.error < 3){
            count++;
        }
        else{
            count = 0;
        }

        auto left = std::max(std::min(left_result.power, 127.0), -127.0) + std::max(std::min(match_result.power, 127.0), -127.0);
        auto right = std::max(std::min(right_result.power, 127.0), -127.0) - std::max(std::min(match_result.power, 127.0), -127.0);

        for(auto& motor : left_drive){
            motor = -(int32_t)left;
        }
        for(auto& motor : right_drive){
            motor = -(int32_t)right;
        }

        pros::delay(5);
    }
}

#define CV_DEADZONE 60

#ifdef ROBOT_TARGET_24
#define CV_X_TARGET 315.0
#define CV_H_TARGET 110
#define CV_BALL_X_TARGET 330
#define CV_BALL_INTAKE_START_RAD 150
#endif //ROBOT_TARGET_24

#ifdef ROBOT_TARGET_15
#define CV_X_TARGET 330
#define CV_H_TARGET 110

#define CV_BALL_X_TARGET 330
#define CV_BALL_INTAKE_START_RAD 150
#endif //ROBOT_TARGET_15

#ifdef ROBOT_TARGET_DEV
#define CV_X_TARGET 0
#define CV_H_TARGET 0
#endif //ROBOT_TARGET_15

//move power between 1 and 127 (inclusive)
#define CV_MOVE_POWER 96

#define CV_PX_TO_DEG 25.0

int goalConst[4];
int ballConst[4];

//TODO - probably refactor
// auto score on the goal
void autoScore(){
	int targetX = goalConst[0];
	int targetY = goalConst[1];
	int targetW = goalConst[2];
	int targetH = goalConst[3];

	if(targetX == 0 && targetY == 0){
		// no goal
		//	halt and try to resolve
		updateMotorGroup(leftDrive, 0);
		updateMotorGroup(rightDrive, 0);

		//ALSO, make sure the camera isnt blocked
		updateMotorGroup(topRollers, -32);
		pros::lcd::print(LCD_AUTO_SCORE_STATUS, "SEARCHING");
		return;
	}

	if (targetH > CV_H_TARGET){
		//we are close enough, any shot will make it
		updateMotorGroup(topRollers, MAX_ROTATION_INTENSITY);
		updateMotorGroup(rollers, MAX_ROTATION_INTENSITY);
		pros::lcd::print(LCD_AUTO_SCORE_STATUS, "SCORE %d %d %d %d", targetX, targetY, targetX, targetH);
	}
	if(std::abs(targetX - CV_X_TARGET) > CV_DEADZONE){
		//turn
        turn(-(CV_X_TARGET - targetX) / CV_PX_TO_DEG, leftDrive, rightDrive);
		pros::lcd::print(LCD_AUTO_SCORE_STATUS, "TURN %d %d %d %d", targetX, targetY, targetX, targetH);
	}else{
		//we know we are aligned, but too far back
		//	so drive forward
		updateMotorGroup(leftDrive, CV_MOVE_POWER);
		updateMotorGroup(rightDrive, CV_MOVE_POWER);
		pros::lcd::print(LCD_AUTO_SCORE_STATUS, "FORWARD %d %d %d %d", targetX, targetY, targetX, targetH);
	}
}

void autoPickup(){
	int targetX = goalConst[0];
	int targetY = goalConst[1];
	int targetR = goalConst[2];
	// int targetC = goalConst[3];

	// pros::lcd::print(0\LCD_AUTO_SCORE_STATUS, "Target: %04d %04d %04d", targetX, targetY, targetR);

	if(targetX == 0 && targetY == 0){
		// no goal
		//	halt and try to resolve
		updateMotorGroup(leftDrive, 0);
		updateMotorGroup(rightDrive, 0);
		pros::lcd::print(LCD_AUTO_SCORE_STATUS, "ball_SEARCHING");
		return;
	}

	if(targetR > CV_BALL_INTAKE_START_RAD){
		updateMotorGroup(intake, MAX_ROTATION_INTENSITY);
		updateMotorGroup(rollers, MAX_ROTATION_INTENSITY);
	}else{
		updateMotorGroup(intake, 0);
		updateMotorGroup(rollers, 0);
	}

	if(std::abs(targetX - CV_BALL_X_TARGET) > CV_DEADZONE){
		//turn
		double turnAmt = -(CV_BALL_X_TARGET - targetX) / CV_PX_TO_DEG;
        turn(turnAmt, leftDrive, rightDrive);
		pros::lcd::print(LCD_AUTO_SCORE_STATUS, "BALL_TURN %d %d %d --> %.03f (%03d)", targetX, targetY, targetR, turnAmt, (CV_BALL_X_TARGET - targetX));
	}else{
		//we know we are aligned, but too far back
		//	so drive forward
		updateMotorGroup(leftDrive, CV_MOVE_POWER);
		updateMotorGroup(rightDrive, CV_MOVE_POWER);
		pros::lcd::print(LCD_AUTO_SCORE_STATUS, "BALLA_FORWARD %d %d %d", targetX, targetY, targetR);
	}
}

void initialize() {
	pros::lcd::initialize();

	//initialize the motors:
	leftDrive.emplace_back(LEFT_DRIVE_MOTOR_1_PORT, LM_1_DIR);
	leftDrive.emplace_back(LEFT_DRIVE_MOTOR_2_PORT, LM_2_DIR);
	leftDrive.emplace_back(LEFT_DRIVE_MOTOR_3_PORT, LM_3_DIR);
#ifdef FOUR_DRIVE_MOTORS
	leftDrive.emplace_back(LEFT_DRIVE_MOTOR_4_PORT, LM_4_DIR);
#endif //FOUR_DRIVE_MOTORS

	rightDrive.emplace_back(RIGHT_DRIVE_MOTOR_1_PORT, RM_1_DIR);
	rightDrive.emplace_back(RIGHT_DRIVE_MOTOR_2_PORT, RM_2_DIR);
	rightDrive.emplace_back(RIGHT_DRIVE_MOTOR_3_PORT, RM_3_DIR);
#ifdef FOUR_DRIVE_MOTORS
	rightDrive.emplace_back(RIGHT_DRIVE_MOTOR_4_PORT, RM_4_DIR);
#endif //FOUR_DRIVE_MOTORS

	intake.emplace_back(LEFT_INTAKE_MOTOR_PORT, LI_DIR);
	intake.emplace_back(RIGHT_INTAKE_MOTOR_PORT, RI_DIR);

#ifndef ROBOT_TARGET_DEV
	rollers.emplace_back(MIDDLE_ROLLER_MOTOR_1_PORT, MR_1_DIR);
	rollers.emplace_back(MIDDLE_ROLLER_MOTOR_2_PORT, MR_2_DIR);

	topRollers.emplace_back(TOP_ROLLER_MOTOR_PORT, TOP_DIR);
#endif

	//in docs, not working
	// IMU.set_reversed(true);
	IMU.reset();

	pros::delay(10);
	do{
		pros::delay(10);
	}while(IMU.is_calibrating());

	pros::Task{
	    [](){
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
            while(true){
                double right_max = 0;
                double left_max = 0;
                for(auto& motor : rightDrive){
                    right_max = std::max(right_max, motor.get_temperature());
                }
                for(auto& motor : leftDrive){
                    left_max = std::max(left_max, motor.get_temperature());
                }
                pros::lcd::print(LCD_OPEN_6, "IMU: %.3f", IMU.get_rotation());
                pros::lcd::print(LCD_OPEN_7, "L/R %.1f/%.1f", left_max, right_max);
                pros::delay(50);
            }
#pragma clang diagnostic pop
        }
	};

	// if(VexMessenger::v_messenger->try_connect(3000)){
	// 	pros::lcd::print(LCD_MESSENGER_CONNECTED, "Messenger Connected.");
	// }else{
	// 	pros::lcd::print(LCD_MESSENGER_CONNECTED, "Messenger Not Connected.");
	// }

#ifdef LOW_POWER_MODE
	pros::lcd::print(LCD_CONST_STATUS, "RUNNING IN LOW POWER MODE");
#else
	pros::lcd::print(LCD_CONST_STATUS, "Normal Operation");
#endif
	
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
[[noreturn]] void disabled() {
	// stores the length of received messages
	uint8_t messageLen;

	// buffer for storing received messages
	uint8_t messageBuffer[MAX_MESSAGE_LEN];

	while(true){
		if(!VexMessenger::v_messenger->isConnected()){
			pros::lcd::print(LCD_MESSENGER_CONNECTED, "MESSENGER DISCONNECTED");
			//try connection non-blocking
			VexMessenger::v_messenger->try_connect(0);
		}else{
			pros::lcd::print(LCD_MESSENGER_CONNECTED, "MESSENGER_CONNECTED");

			//see if there is a message to process
			VexMessenger::v_messenger->readDataMessage(messageBuffer, messageLen, 0);
			
			//don't do processing, just destroy messages
		}

		pros::delay(50);
	}
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	pros::lcd::print(LCD_LOCAL_STATUS, "Starting OP Control");

	//primary controller object
	pros::Controller master(pros::E_CONTROLLER_MASTER);

	// stores received message length
	uint8_t messageLen;
	
	// buffer for received message
	uint8_t messageBuffer[MAX_MESSAGE_LEN];

	// keep a count of how many times the loop has run
	uint32_t loop_counter = 0;

	int32_t ctr = 50000;

	int32_t lastDrivePower_right = 0;
	int32_t lastDrivePower_left = 0;

	while(true){
		loop_counter++;
		pros::lcd::print(LCD_LOCAL_STATUS, "Loop %d checkpoint A", loop_counter);

		// messenger based control
		if(!VexMessenger::v_messenger->isConnected()){
			pros::lcd::print(LCD_MESSENGER_CONNECTED, "MESSENGER DISCONNECTED");
			//try connection non-blocking
			VexMessenger::v_messenger->try_connect(0);
		}else{
			pros::lcd::print(LCD_MESSENGER_CONNECTED, "MESSENGER_CONNECTED");

			//see if there is a message to process

			//used to keep this loop from being greedy and blocking
			int msgCounter = 4;
			while(msgCounter-- && VexMessenger::v_messenger->readDataMessage(messageBuffer, messageLen, 0)){
				processMessage(messageBuffer, messageLen);
			}
		}

		pros::lcd::print(LCD_LOCAL_STATUS, "Loop %d checkpoint B", loop_counter);

		// joystick based control
		if(master.is_connected()){
			// set to true when autoscoring to prevent unintentional interference
			bool triedAuto = false;
			if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X)){
				pros::lcd::print(LCD_AUTO_SCORE_STATUS, "--clr--");
			}

			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_X)){
				triedAuto = true;
				autoScore();
			}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_B)){
				triedAuto = true;
				autoPickup();
			}

#ifdef DRIVER_HUMZA
			int32_t left_y = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
			int32_t right_y = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
			left_y = std::max(left_y, lastDrivePower_left - 10);
			right_y = std::max(right_y, lastDrivePower_right - 10);
			tankDrive(left_y, right_y, !triedAuto);
			lastDrivePower_right = right_y;
			lastDrivePower_left = left_y;
#else 
			int32_t arcade_y = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
			int32_t arcade_x = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
			arcadeDrive(arcade_x, arcade_y, !triedAuto);
#endif // DRIVER_HUMZA

#ifndef DRIVER_TRENT 
			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)){
				updateMotorGroup(intake, MAX_ROTATION_INTENSITY);
			}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)){
				updateMotorGroup(intake, -MAX_ROTATION_INTENSITY);
			}else if(!triedAuto){
				updateMotorGroup(intake, 0);
			}

			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_A)){
				updateMotorGroup(topRollers, MAX_ROTATION_INTENSITY);
			}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_Y)){
				updateMotorGroup(topRollers, -MAX_ROTATION_INTENSITY);
			}else if(!triedAuto){
				updateMotorGroup(topRollers, 0);
			}

			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)){
				updateMotorGroup(rollers, MAX_ROTATION_INTENSITY*.7);
			}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)){
				updateMotorGroup(rollers, -MAX_ROTATION_INTENSITY*.7);
#ifdef DRIVER_HUMZA
				updateMotorGroup(topRollers, -MAX_ROTATION_INTENSITY);
#endif //DRIVER_HUMZA
			}else if(!triedAuto){
				updateMotorGroup(rollers, 0);
			}

			if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_LEFT)){
				//rotate left 90
                turn(-90, leftDrive, rightDrive);
			}else if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_RIGHT)){
                //rotate right 90
                turn(90, leftDrive, rightDrive);
            }
			else if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_UP)){
                drive_for_distance(1000, leftDrive, rightDrive);
            }
			else if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN)){
                drive_for_distance(-1000, leftDrive, rightDrive);
            }

#else // DRIVER_TRENT

#define INTAKE_MASK 1
#define ROLLER_MASK 2
#define TOP_MASK 4

		int mask = 0;

		// updateMotorGroup(topRollers, -0);
		// updateMotorGroup(rollers, -0);
		// updateMotorGroup(intake, -0);

		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)){
			if(!(mask & ROLLER_MASK)){
			updateMotorGroup(rollers, MAX_ROTATION_INTENSITY);
			}
			if(!(mask & TOP_MASK)){
			updateMotorGroup(topRollers, MAX_ROTATION_INTENSITY);
			}

			mask |= ROLLER_MASK | TOP_MASK;
		}

		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)){
			if(!(mask & TOP_MASK)){
				updateMotorGroup(topRollers, -MAX_ROTATION_INTENSITY*.5);
			}
			if(!(mask & ROLLER_MASK)){
				updateMotorGroup(rollers, MAX_ROTATION_INTENSITY);
			}

			if(!(mask & INTAKE_MASK)){
				updateMotorGroup(intake, MAX_ROTATION_INTENSITY);
			}
			mask |= ROLLER_MASK | TOP_MASK | INTAKE_MASK;
		}

		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)){
			if(!(mask & TOP_MASK)){
				updateMotorGroup(topRollers, -MAX_ROTATION_INTENSITY);
			}
			if(!(mask & ROLLER_MASK)){
				updateMotorGroup(rollers, -MAX_ROTATION_INTENSITY);
			}

			if(!(mask & INTAKE_MASK)){
				updateMotorGroup(intake, -MAX_ROTATION_INTENSITY);
			}
			mask |= ROLLER_MASK | TOP_MASK | INTAKE_MASK;
		}

		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)){
			updateMotorGroup(topRollers, -MAX_ROTATION_INTENSITY);
			updateMotorGroup(rollers, -MAX_ROTATION_INTENSITY);
			mask |= ROLLER_MASK | TOP_MASK;
		}

		if(!(mask & 1)){
			updateMotorGroup(intake, 0);
		}
		if(!(mask & 2)){
			updateMotorGroup(rollers, 0);
		}
		if(!(mask & 4)){
			updateMotorGroup(topRollers, 0);
		}
#endif // DRIVER_TRENT
		} // if controller is connected

		pros::lcd::print(LCD_LOCAL_STATUS, "Loop %d checkpoint Z", loop_counter);
		pros::delay(10);
	}
}
