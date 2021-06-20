#include "main.h"
#include "vexController.h"
// #include "vexSerial.h"
#include "vexMessenger.h"
#include "util.h"

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
pros::Imu IMU(IMU_PORT);

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
				pros::lcd::print(LCD_OPEN_5, "MESSAGE PROCESSING CURRENTLY DISABLED");
			}
		}

		pros::lcd::print(LCD_LOCAL_STATUS, "Loop %d checkpoint B", loop_counter);

		// joystick based control
		if(master.is_connected()){
			int32_t arcade_y = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
			int32_t arcade_x = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
			arcadeDrive(arcade_x, arcade_y);

			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)){
				updateMotorGroup(intake, MAX_ROTATION_INTENSITY);
			}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)){
				updateMotorGroup(intake, -MAX_ROTATION_INTENSITY);
			}else{
				updateMotorGroup(intake, 0);
			}

			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)){
				updateMotorGroup(rollers, MAX_ROTATION_INTENSITY);
			}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)){
				updateMotorGroup(rollers, -MAX_ROTATION_INTENSITY);
			}else{
				updateMotorGroup(rollers, 0);
			}

			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_A)){
				updateMotorGroup(topRollers, MAX_ROTATION_INTENSITY);
			}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_Y)){
				updateMotorGroup(topRollers, -MAX_ROTATION_INTENSITY);
			}else{
				updateMotorGroup(topRollers, 0);
			}
		}


		pros::lcd::print(LCD_LOCAL_STATUS, "Loop %d checkpoint Z", loop_counter);
		pros::delay(50);
	}

// 	uint8_t nextLine = 4;
// 	uint8_t msgLen;
// 	uint8_t recvBuffer[MAX_MESSAGE_LEN];

// 	// used for turning controller on and off
// 	bool controllerOn = false;

// 	// used for making the on off more reliable
// 	//	forces a realease before toggling again
// 	bool lastPress = false;

// 	char upmsg[]    = "up-pressed";
// 	char leftmsg[]  = "left-pressed";
// 	char rightmsg[] = "right-pressed";
// 	char downmsg[]  = "down-pressed";

// 	master.print(0, 0, "%s", "CTRL DISABLED");

// 	pros::lcd::print(1, "VEX_Messenger ---");
// #ifdef LOW_POWER_MODE
// 		pros::lcd::print(0, "LOW POWER MODE");
// 		master.print(1, 0, "%s", "LOW POWER MODE");
// #else
// 		pros::lcd::print(0,"Normal Operation");
// #endif

// 	while (true) {
// 		// if(VexSerial::v_ser->receiveMessageIfAvailable(recvBuffer, msgLen))
// 		// {
// 		// 	pros::lcd::print(
// 		// 		nextLine, "(%03d) %02X %02X %02X %02X  %02X %02X %02X %02X",
// 		// 		msgLen, 
// 		// 		recvBuffer[0], recvBuffer[1],
// 		// 		recvBuffer[2], recvBuffer[3],
// 		// 		recvBuffer[4], recvBuffer[5],
// 		// 		recvBuffer[6], recvBuffer[7]
// 		// 	);

// 		// 	nextLine = (nextLine + 1); // fast mod8
// 		// 	if(nextLine >= 8){
// 		// 		nextLine = 4;
// 		// 	}

// 		// 	VexSerial::v_ser->sendMessage(recvBuffer, msgLen);
// 		// }else{
// 		// 	pros::lcd::print(0, "VSER_false");
// 		// }

// 		// pros::delay(20);

// 		if(VexMessenger::v_messenger->isConnected()){
// 			pros::lcd::print(1, "VEX_Messenger Connected!");

// 			if(VexMessenger::v_messenger->readDataMessage(recvBuffer, msgLen, 50)){
// 				recvBuffer[msgLen] = 0; //add null terminator
// 				pros::lcd::print(6, "%d: %02X %02X %02X %02X  %02X %02X %02X %02X", msgLen, recvBuffer[0], recvBuffer[1], recvBuffer[2], recvBuffer[3], recvBuffer[4], recvBuffer[5], recvBuffer[6], recvBuffer[7]);
// 				processMessage(recvBuffer, msgLen);
// 			}
// 		}
// 		else{
// 			pros::lcd::print(1, "VEX_Messenger %s",
// 				((VexMessenger::v_messenger->try_connect(200)) ? ("Connection Success!") : ("Connection Failed."))
// 			);
// 		}

// 		if(master.is_connected()){
// 			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_UP)){
// 				VexMessenger::v_messenger->sendMessage((const uint8_t*)(upmsg), strlen(upmsg));
// 			}
// 			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT)){
// 				VexMessenger::v_messenger->sendMessage((const uint8_t*)(leftmsg), strlen(leftmsg));
// 			}
// 			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT)){
// 				VexMessenger::v_messenger->sendMessage((const uint8_t*)(rightmsg), strlen(rightmsg));
// 			}
// 			if(master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN)){
// 				VexMessenger::v_messenger->sendMessage((const uint8_t*)(downmsg), strlen(downmsg));
// 			}

// 			if(!lastPress && master.get_digital(pros::E_CONTROLLER_DIGITAL_X)){
// 				controllerOn = !controllerOn;
// 				master.print(0, 0, "%s", (controllerOn ? "CTRL ENABLED " : "CTRL DISABLED"));
// 				lastPress = true;
// 			}else if(lastPress && !master.get_digital(pros::E_CONTROLLER_DIGITAL_X)){
// 				lastPress = false;
// 			}

// 			pros::lcd::print(4, "Controller Connected: %s", (controllerOn ? "ON" : "OFF"));

// 			if(controllerOn){
// 				int32_t leftY = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
// 				int32_t rightY = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
// 				updateDrive(leftY, rightY);

// 				if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)){
// 					updateMotorGroup(intake, 127);
// 				}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)){
// 					updateMotorGroup(intake, -127);
// 				}else{
// 					updateMotorGroup(intake, 0);
// 				}

// 				if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)){
// 					updateMotorGroup(rollers, 127);
// 				}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)){
// 					updateMotorGroup(rollers, -127);
// 				}else{
// 					updateMotorGroup(rollers, 0);
// 				}
// 			}
// 		}else{
// 			pros::lcd::print(4, "Controller Disconnected");
// 		}

// 		pros::lcd::print(5, "IMU reading: %f", -IMU.get_rotation());
// 	}
}
