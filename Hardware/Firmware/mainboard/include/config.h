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

const offsets mpu_offsets[] = {
    {-262, -2994, 1711, -174, 214, 46},
    {-6018, 1394, 1385, 66, -86, 35},
    {-3296, 434, 1879, 377, -178, -6},
    {-889, -4586, 1050, 117, -242, 54},
    {2071, -3990, 1582, -42, 119, 58},
    {-2539, -2304, 372, 90, 3, 18},
};
