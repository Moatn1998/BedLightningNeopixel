#pragma once

/*
DEBUG_MODE :         + If defined, a lot of debug-information is printed to the serial monitor 
*/
// #define DEBUG_MODE

/*
TEST_MODE :         + PIRs are not blocked during the day (information of time is taken from the internet)
                    + Whole get time routine is deactivated
                    + Duration of active light due to PIRs is reduced
*/
// #define TEST_MODE


/*
LOOP_TIME :         + Information about the time it takes for a defined number of iterations of loop() to perform is monitored
*/
// #define LOOP_TIME


/*
BLOCK_PIR_XXXX :    + Define one of this options so that the corresponding PIR is ignored when it triggers
*/
// #define BLOCK_PIR_MARTIN
// #define BLOCK_PIR_FUSSTEIL
#define BLOCK_PIR_NADJA


// defined animations of the neopixel strip
enum SELECTED_ANIMATION
{
RGB,                        // 0
WEISS,                      // 1
COLOR_WIPE,                 // 2
THEATER_CHASE,              // 3
RAINBOW,                    // 4
THEATER_CHASE_RAINBOW,      // 5
PUFF_MODUS,                 // 6
CYLON_BOUNCE,               // 7
SPARKLE,                    // 8
RUNNING_LIGHTS,             // 9 
FIRE,                       // 10
BOUNCING_BALLS,             // 11
BOUNCING_BALLS_COLORED,     // 12
METEOR_RAIN,                // 13
FIRE_ANIMATION,             // 14
};