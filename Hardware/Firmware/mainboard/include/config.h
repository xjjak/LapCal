// -----DEFINITIONS---
#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 5

#define LED_GREEN 33
#define LED_BLUE 13

#define SDA 21
#define SCL 22

#define SD_DETECT 17

#define TOUCH_PIN 32

#define VBAT_SENSE 35

#define SENSOR_COUNT 6
//---------------------

// THRESHHOLDS
#define THRESH_LOW_BAT 1800
#define THRESH_TOUCH 15
#define THRESH_NO_TOUCH 30

struct reading {
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
};

struct offsets {
    int xa, ya, za, xg, yg, zg;
};

#ifndef RIGHT_HAND
#error Specify hand.
#endif

#if RIGHT_HAND
    const offsets mpu_offsets[] = {
        {-262, -2994, 1711, -174, 214, 46}, // Thumb
        {-6018, 1394, 1385, 66, -86, 35}, // Index
        {-3296, 434, 1879, 377, -178, -6}, // Middle
        {-889, -4586, 1050, 117, -242, 54}, // Ring 
        {2071, -3990, 1582, -42, 119, 58}, // Pinky
        {-2539, -2304, 372, 90, 3, 18}, // Onboard
    };
#else
    // Remember: this config is also in reverse order if sensors_array is switched
    const offsets mpu_offsets[] = {
        {-3571, -2403, 3423, 208, -90, -214}, // Thumb
        {-1260, 3656, 6252, -3360, -493, 31}, // Index
        {-15598, -3882, 5878, -55, -292, 297}, // Middle
        {-15598, -3882, 5878, -55, -292, 297}, // Ring
        {-368, -18006, 7812, 132, -141, -140}, // Pinky
        {-1074, 2309, 1022, 116, -26, 26}, // Onboard
    };
#endif
