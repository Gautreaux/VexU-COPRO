#include "main.h"
#include "util.h"

enum class COMMAND_ENUM {
    STOP = 0,
    TEXT = 1,
    JITTER = 10,
    START_ROTATION_LEFT = 11,
    START_ROTATION_RIGHT = 12,
};

// how fast should start/stop rotation run
//  something in range 0-127
//      but you know, dont do 0, it wont do anything
#define ROTATION_INTENSITY 30

// process a single message instance
void processMessage(uint8_t const  * const buffer, const uint8_t len);

// stop all robot motion
void stop(uint8_t const  * const buffer, const uint8_t len);

// display text to the screen
void text(uint8_t const  * const buffer, const uint8_t len);

void jitter(uint8_t const  * const buffer, const uint8_t len);

void startRotation(uint8_t const  * const buffer, const uint8_t len, const bool isRotationLeft = true);