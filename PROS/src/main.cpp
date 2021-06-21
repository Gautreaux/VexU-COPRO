#include "main.h"
#include "vexController.h"
// #include "vexSerial.h"
#include "vexMessenger.h"
#include "util.h"
#include "spencerPID.h"

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
pros::Imu IMU(IMU_PORT);

#define CV_DEADZONE 60

#ifdef ROBOT_TARGET_24
// These need to be recalibrated
// #define CV_X_TARGET 270
// #define CV_H_TARGET 25
#endif //ROBOT_TARGET_24

#ifdef ROBOT_TARGET_15
#define CV_X_TARGET 330
#define CV_H_TARGET 110
#endif //ROBOT_TARGET_15

//move power between 1 and 127 (inclusive)
#define CV_MOVE_POWER 96

#define CV_PX_TO_DEG 25

int goalConst[4];

//TODO - probably refactor
// auto score on the goal
void autoScore(void){
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
	if(abs(targetX - CV_X_TARGET) > CV_DEADZONE){
		//turn
		if(!SpencerPID::isPIDRunning()){
			SpencerPID::rotateDegrees((CV_X_TARGET - targetX) / CV_PX_TO_DEG);
		}
		pros::lcd::print(LCD_AUTO_SCORE_STATUS, "TURN %d %d %d %d", targetX, targetY, targetX, targetH);
	}else{
		//we know we are aligned, but too far back
		//	so drive forward
		updateMotorGroup(leftDrive, CV_MOVE_POWER);
		updateMotorGroup(rightDrive, CV_MOVE_POWER);
		pros::lcd::print(LCD_AUTO_SCORE_STATUS, "FORWARD %d %d %d %d", targetX, targetY, targetX, targetH);
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

#ifndef ROBOT_TARGET_DEV
	intake.emplace_back(LEFT_INTAKE_MOTOR_PORT, LI_DIR);
	intake.emplace_back(RIGHT_INTAKE_MOTOR_PORT, RI_DIR);

	rollers.emplace_back(MIDDLE_ROLLER_MOTOR_1_PORT, MR_1_DIR);
	rollers.emplace_back(MIDDLE_ROLLER_MOTOR_2_PORT, MR_2_DIR);

	topRollers.emplace_back(TOP_ROLLER_MOTOR_PORT, TOP_DIR);
#endif

	//in docs, not working
	// IMU.set_reversed(true);
	IMU.reset();

	if(VexMessenger::v_messenger->try_connect(3000)){
		pros::lcd::print(LCD_MESSENGER_CONNECTED, "Messenger Connected.");
	}else{
		pros::lcd::print(LCD_MESSENGER_CONNECTED, "Messenger Not Connected.");
	}

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
void disabled() {
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
			}

#ifdef DRIVER_AARON
			int32_t arcade_y = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
			int32_t arcade_x = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
			arcadeDrive(arcade_x, arcade_y, !triedAuto);
#endif
#ifdef DRIVER_HUMZA
			int32_t left_y = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
			int32_t right_y = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
			tankDrive(left_y, right_y, !triedAuto);
#endif

			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)){
				updateMotorGroup(intake, MAX_ROTATION_INTENSITY);
			}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)){
				updateMotorGroup(intake, -MAX_ROTATION_INTENSITY);
			}else if(!triedAuto){
				updateMotorGroup(intake, 0);
			}

			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)){
				updateMotorGroup(rollers, MAX_ROTATION_INTENSITY);
			}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)){
				updateMotorGroup(rollers, -MAX_ROTATION_INTENSITY);
#ifdef DRIVER_HUMZA
				updateMotorGroup(intake, -MAX_ROTATION_INTENSITY);
#endif //DRIVER_HUMZA
			}else if(!triedAuto){
				updateMotorGroup(rollers, 0);
			}

			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_A)){
				updateMotorGroup(topRollers, MAX_ROTATION_INTENSITY);
			}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_Y)){
				updateMotorGroup(topRollers, -MAX_ROTATION_INTENSITY);
			}else if(!triedAuto){
				updateMotorGroup(topRollers, 0);
			}

			if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_LEFT)){
				//rotate left 90
				SpencerPID::rotateDegrees(90);
			}else if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_RIGHT)){
				//rotate right 90
				SpencerPID::rotateDegrees(-90);
			}
		}

		SpencerPID::updatePID();

		pros::lcd::print(LCD_LOCAL_STATUS, "Loop %d checkpoint Z", loop_counter);
		pros::delay(50);
	}
}