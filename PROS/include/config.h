#ifndef __CONFIG__
#define __CONFIG__

// enable flag for extreme low power operation
// #define LOW_POWER_MODE

// universal stuff
#define PORT_UNKNOWN 30
#define CONTROLLER_THRESHOLD 20

// how fast should start/stop rotation run
//  something in range 0-127
//      but you know, dont do 0, it wont do anything
#ifdef LOW_POWER_MODE
#define ROTATION_INTENSITY 30
#else
#define ROTATION_INTENSITY 127
#endif

#define SLOW_ROTATION_INTENSITY 30
#define MAX_ROTATION_INTENSITY 127

// constants for which line is which on the screen
#define LCD_CONST_STATUS 0
#define LCD_MESSENGER_CONNECTED 1
#define LCD_SENDER_STATUS 2
#define LCD_RECEIVER_STATUS 3
#define LCD_LOCAL_STATUS 4
#define LCD_AUTO_SCORE_STATUS 5
#define LCD_OPEN_6 6
#define LCD_OPEN_7 7


// specify which robot is being target with this build
// #define ROBOT_TARGET_DEV
// #define ROBOT_TARGET_24
#define ROBOT_TARGET_15


// specify which driver is running this code
// #define DRIVER_AARON
#define DRIVER_HUMZA
// #define DRIVER_TRENT


#ifdef ROBOT_TARGET_DEV

#define FOUR_DRIVE_MOTORS

#define RIGHT_DRIVE_MOTOR_1_PORT 2
#define RIGHT_DRIVE_MOTOR_2_PORT 4
#define RIGHT_DRIVE_MOTOR_3_PORT 3
#define RIGHT_DRIVE_MOTOR_4_PORT 1

#define LEFT_DRIVE_MOTOR_1_PORT 14
#define LEFT_DRIVE_MOTOR_2_PORT 13
#define LEFT_DRIVE_MOTOR_3_PORT 12
#define LEFT_DRIVE_MOTOR_4_PORT 11

#define LM_1_DIR false
#define LM_2_DIR false
#define LM_3_DIR false
#define LM_4_DIR false

#define RM_1_DIR true
#define RM_2_DIR true
#define RM_3_DIR true
#define RM_4_DIR true

#define LEFT_INTAKE_MOTOR_PORT 16
#define RIGHT_INTAKE_MOTOR_PORT 17

#define LI_DIR false
#define RI_DIR true

#define IMU_PORT 19

#endif //ROBOT TARGET DEV

#ifdef ROBOT_TARGET_15
#define RIGHT_DRIVE_MOTOR_1_PORT 12
#define RIGHT_DRIVE_MOTOR_2_PORT 14
#define RIGHT_DRIVE_MOTOR_3_PORT 13

#define LEFT_DRIVE_MOTOR_1_PORT 4
#define LEFT_DRIVE_MOTOR_2_PORT 3
#define LEFT_DRIVE_MOTOR_3_PORT 2

#define LEFT_INTAKE_MOTOR_PORT 5
#define RIGHT_INTAKE_MOTOR_PORT 15

#define MIDDLE_ROLLER_MOTOR_1_PORT 17
#define MIDDLE_ROLLER_MOTOR_2_PORT 18
#define TOP_ROLLER_MOTOR_PORT 8

#define IMU_PORT 20

#define LM_1_DIR true
#define LM_2_DIR true
#define LM_3_DIR true

#define RM_1_DIR false
#define RM_2_DIR false
#define RM_3_DIR false

#define LI_DIR false
#define RI_DIR true

#define MR_1_DIR false
#define MR_2_DIR false

#define TOP_DIR false
#endif //ROBOT_TARGET_15

#ifdef ROBOT_TARGET_24

#define RIGHT_DRIVE_MOTOR_1_PORT 2
#define RIGHT_DRIVE_MOTOR_2_PORT 4
#define RIGHT_DRIVE_MOTOR_3_PORT 3

#define LEFT_DRIVE_MOTOR_1_PORT 14
#define LEFT_DRIVE_MOTOR_2_PORT 13
#define LEFT_DRIVE_MOTOR_3_PORT 12

#define LEFT_INTAKE_MOTOR_PORT 15
#define RIGHT_INTAKE_MOTOR_PORT 16

#define MIDDLE_ROLLER_MOTOR_1_PORT 7
#define MIDDLE_ROLLER_MOTOR_2_PORT 8
#define TOP_ROLLER_MOTOR_PORT 5

#define IMU_PORT 20

#define LM_1_DIR true
#define LM_2_DIR true
#define LM_3_DIR true

#define RM_1_DIR false
#define RM_2_DIR false
#define RM_3_DIR false

#define LI_DIR true
#define RI_DIR false

#define MR_1_DIR true
#define MR_2_DIR true

#define TOP_DIR false
#endif // ROBOT_TARGET_24

#ifdef DRIVER_AARON
#endif // DRIVER_AARON

#ifdef DRIVER_HUMZA
#endif // DRIVER_HUMZA

#ifdef DRIVER_TRENT
#error "Trent has not configured his drive style."
#endif // DRIVER_TRENT


//safety checks to ensure that only one target is specified
#ifdef ROBOT_TARGET_DEV
#ifdef ROBOT_TARGET_24
#error "Cannot specify multiple robot targets"
#endif
#endif

#ifdef ROBOT_TARGET_DEV
#ifdef ROBOT_TARGET_15
#error "Cannot specify multiple robot targets"
#endif
#endif

#ifdef ROBOT_TARGET_15
#ifdef ROBOT_TARGET_24
#error "Cannot specify multiple robot targets"
#endif
#endif

//safety checks to ensure that at least one target is specified
#ifndef ROBOT_TARGET_DEV
#ifndef ROBOT_TARGET_24
#ifndef ROBOT_TARGET_15
#error "Must specify at least one robot target"
#endif
#endif
#endif

// safety to check that only one driver is specified
#ifdef DRIVER_AARON
#ifdef DRIVER_HUMZA
#error "Must specify only one driver"
#endif // DRIVER_HUMZA
#endif // DRIVER_AARON

#ifdef DRIVER_TRENT
#ifdef DRIVER_HUMZA
#error "Must specify only one driver"
#endif // DRIVER_HUMZA
#endif // DRIVER_TRENT

#ifdef DRIVER_AARON
#ifdef DRIVER_TRENT
#error "Must specify only one driver"
#endif // DRIVER_TRENT
#endif // DRIVER_AARON

//safety check to ensure that at least one driver is specified
#ifndef DRIVER_AARON
#ifndef DRIVER_TRENT
#ifndef DRIVER_HUMZA
#error "Must specify at least one driver"
#endif // DRIVER_HUMZA
#endif // DRIVER_TRENT
#endif // DRIVER_AARON

#endif //__CONFIG__