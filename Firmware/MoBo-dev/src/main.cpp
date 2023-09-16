#include <Arduino.h>

// #include <i2cdev.h>
// #include <MPU6050.h>
#include "MPU6050_6Axis_MotionApps20.h"

#include "Wire.h"


#define LED_GREEN 33
#define LED_BLUE 13

#define SDA 21
#define SCL 22

#define SENSOR_COUNT 5

MPU6050 sense1;


// MPU control/status vars
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector


// Select I2C BUS
void tca_select(uint8_t bus){
  Wire.beginTransmission(0x70);  // tca_select address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}


void setup_mpu(MPU6050 mpu, int id) {
    tca_select(id);
    mpu.initialize();

    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXAccelOffset(-53); // 1688 factory default for my test chip
    mpu.setYAccelOffset(-3033); // 1688 factory default for my test chip
    mpu.setZAccelOffset(1723); // 1688 factory default for my test chip
    mpu.setXGyroOffset(-173);
    mpu.setYGyroOffset(216);
    mpu.setZGyroOffset(45);

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);


    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
    } 

}

void readFifoBuffer(MPU6050 mpu) {

    // Serial.println("Reading fifo buffer...");
    // Clear the buffer so as we can get fresh values
    // The sensor is running a lot faster than our sample period
    mpu.resetFIFO();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);
    // Serial.println("Read buffer.");
}

void sense_readings(MPU6050 mpu) {
    readFifoBuffer(sense1);
    // display real acceleration, adjusted to remove gravity
    // Serial.println("Getting vals");
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
    float x = aaReal.x;
    float y = aaReal.y;
    float z = aaReal.z;

    // Serial.println("here they are:");
    //
    char buf[100];
    sprintf(buf,"%.2f;%.2f;%.2f", x,y,z);
    // Serial.println(x);
    // Serial.println(y);
    // Serial.println(z);
    Serial.println(buf);
}


void setup() {
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_BLUE, HIGH);
  Serial.begin(115200);

  // ------SETUP SD------------
  // setup_SD();

  Serial.println("Initialising i2c");
  Wire.begin(SDA, SCL); // Setup i2c channel on defined pins

  setup_mpu(sense1, 0);



}


void loop() {
  sense_readings(sense1);
  delay(200);
}
