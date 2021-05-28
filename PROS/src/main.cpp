#include "main.h"
#include "vexSerial.h"

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");
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
 * Runs reading of code
 */
void readCallback(const uint8_t * const buff, const uint8_t sz) {
	pros::lcd::print(6, "(%d) %s", sz, buff);
	pros::delay(1500);
}

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

	pros::ADIDigitalIn sensor('A');
	int32_t ctr = 0;

	VexSerial::v_ser->setCallback(readCallback);
	VexSerial::v_ser->tryConnect();
	pros::lcd::print(5, "Last Serial Message:");

	char buffer[STREAM_BUFFER_SZ];
	memset(buffer, 0, STREAM_BUFFER_SZ);

	while (true) {
		pros::lcd::print(0, "%d %d %d", (pros::lcd::read_buttons() & LCD_BTN_LEFT) >> 2,
		                 (pros::lcd::read_buttons() & LCD_BTN_CENTER) >> 1,
		                 (pros::lcd::read_buttons() & LCD_BTN_RIGHT) >> 0);

		pros::lcd::print(2, "Phys Button: %d", sensor.get_value());

		if(VexSerial::v_ser->isConnected()){
			pros::lcd::print(7, "Client Connected...");
		}else{
			pros::lcd::print(7, "NO CLIENT!!!");
			VexSerial::v_ser->tryConnect();
		}

		if(++ctr % 100 == 0){
			pros::lcd::print(3, "%d", ctr);
			// sprintf(buffer, "%d\n", ctr);
			// VexSerial::v_ser->sendData((uint8_t*)buffer, strlen(buffer));
		}

		pros::delay(20);
	}
}
