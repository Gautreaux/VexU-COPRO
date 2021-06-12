#include "main.h"
// #include "vexSerial.h"
#include "vexMessenger.h"
#include "util.h"

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();

	//initialize the motors:
	//TODO - does the gearing/encoder setting matter?
	leftDrive.emplace_back(LEFT_DRIVE_MOTOR_1_PORT, true);
	leftDrive.emplace_back(LEFT_DRIVE_MOTOR_2_PORT, true);
	leftDrive.emplace_back(LEFT_DRIVE_MOTOR_3_PORT, true);

	rightDrive.emplace_back(RIGHT_DRIVE_MOTOR_1_PORT, false);
	rightDrive.emplace_back(RIGHT_DRIVE_MOTOR_2_PORT, false);
	rightDrive.emplace_back(RIGHT_DRIVE_MOTOR_3_PORT, false);

	intake.emplace_back(LEFT_INTAKE_MOTOR_PORT, false);
	intake.emplace_back(RIGHT_INTAKE_MOTOR_PORT, true);

	rollers.emplace_back(MIDDLE_ROLLER_MOTOR_1_PORT, false);
	rollers.emplace_back(MIDDLE_ROLLER_MOTOR_2_PORT, false);
	rollers.emplace_back(TOP_ROLLER_MOTOR_PORT, false);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

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
	pros::Controller master(pros::E_CONTROLLER_MASTER);

	uint8_t nextLine = 4;
	uint8_t msgLen;
	uint8_t recvBuffer[200];

#ifdef DEBUG_PROS
	pros::lcd::print(0, "PROS running in DEBUG_PROS");
#endif

	pros::lcd::print(1, "VEX_Messenger ---");

	char myMessage[] = "apple";

	std::string messages[] = {"apple", "banana", "cherry", "date"};

	while (true)
	{
		for(unsigned int i = 0; i < 4; i++){
			VexMessenger::v_messenger->sendMessage((const uint8_t*)messages[i].c_str(), messages[i].length());
		}

		// printf("APPLE\n");
		// pros::delay(1000);
		// printf("BANANA\n");
		// pros::delay(1000);
		// printf("CHERRY\n");
		// pros::delay(1000);
		// printf("DATE\n");
		// pros::delay(1000);
	}
	

	while (true)
	{

		// if(VexMessenger::v_messenger->isConnected()){
		// 	pros::lcd::print(1, "VEX_Messenger connected");
		// 	VexMessenger::v_messenger->sendMessage((uint8_t*)myMessage, strlen(myMessage) + 1);
		// 	pros::delay(500);
		// }else{
		// 	if(VexMessenger::v_messenger->tryConnect()){
				
		// 	}else{
		// 		pros::lcd::print(1, "VEX_Messenger connection failed");
		// 	}
		// }
	}
	

	// while (true) {
	// 	pros::lcd::print(0,"Hello");
	// 	// if(VexSerial::v_ser->receiveMessageIfAvailable(recvBuffer, msgLen))
	// 	// {
	// 	// 	pros::lcd::print(
	// 	// 		nextLine, "(%03d) %02X %02X %02X %02X  %02X %02X %02X %02X",
	// 	// 		msgLen, 
	// 	// 		recvBuffer[0], recvBuffer[1],
	// 	// 		recvBuffer[2], recvBuffer[3],
	// 	// 		recvBuffer[4], recvBuffer[5],
	// 	// 		recvBuffer[6], recvBuffer[7]
	// 	// 	);

	// 	// 	nextLine = (nextLine + 1); // fast mod8
	// 	// 	if(nextLine >= 8){
	// 	// 		nextLine = 4;
	// 	// 	}

	// 	// 	VexSerial::v_ser->sendMessage(recvBuffer, msgLen);
	// 	// }else{
	// 	// 	pros::lcd::print(0, "VSER_false");
	// 	// }

	// 	// pros::delay(20);

	// 	if(VexMessenger::v_messenger->isConnected()){
	// 		pros::lcd::print(1, "VEX_Messenger Connected!");

	// 		if(VexMessenger::v_messenger->readDataMessage(recvBuffer, msgLen, 50)){
	// 			recvBuffer[msgLen] = 0; //add null terminator
	// 			pros::lcd::print(6, "%s", recvBuffer);
	// 		}
	// 	}
	// 	else{
	// 		pros::lcd::print(1, "VEX_Messenger %s",
	// 			((VexMessenger::v_messenger->try_connect(200)) ? ("Connection Success!") : ("Connection Failed."))
	// 		);
	// 	}

	// 	if(master.is_connected()){
	// 		pros::lcd::print(4, "Controller Connected");

	// 		int32_t leftY = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
	// 		int32_t rightY = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
	// 		updateDrive(leftY, rightY);

	// 		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)){
	// 			updateMotorGroup(intake, 127);
	// 		}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)){
	// 			updateMotorGroup(intake, -127);
	// 		}else{
	// 			updateMotorGroup(intake, 0);
	// 		}

	// 		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)){
	// 			updateMotorGroup(rollers, 127);
	// 		}else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)){
	// 			updateMotorGroup(rollers, -127);
	// 		}else{
	// 			updateMotorGroup(rollers, 0);
	// 		}
	// 	}
	// }
}
