#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID "f5aea478-9ec3-4bcf-af20-7f75e7c68c9d"
#define CHARACTERISTIC_UUID "68bf07fb-d00b-4c80-a796-f8be82b5dea7"

// #include <i2cdev.h>
// #include <MPU6050.h>
#include "MPU6050_6Axis_MotionApps20.h"

#include "Wire.h"


#define LED_GREEN 33
#define LED_BLUE 13

#define SDA 23
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

byte reading_data_array[12];

BLEServer *pServer;
BLEService *pService;
BLECharacteristic* pCharacteristic;


// Select I2C BUS
void tca_select(uint8_t bus){
  Wire.beginTransmission(0x70);  // tca_select address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

void format_reading(byte *reading_data, uint16_t ax, uint16_t ay, uint16_t az,uint16_t gx,uint16_t gy,uint16_t gz){
  //byte reading_data[12];

  reading_data[0] = ax >> 8 & 0xFF;
  reading_data[1] = ax & 0xFF;
  reading_data[2] = ay >> 8 & 0xFF;
  reading_data[3] = ay & 0xFF;
  reading_data[4] = az >> 8 & 0xFF;
  reading_data[5] = az & 0xFF;
  reading_data[6] = gx >> 8 & 0xFF;
  reading_data[7] = gx & 0xFF;
  reading_data[8] = gy >> 8 & 0xFF;
  reading_data[9] = gy & 0xFF;
  reading_data[10] = gz >> 8 & 0xFF;
  reading_data[11] = gz & 0xFF;
  

  
}


void setup_mpu(MPU6050 mpu, int id) {
  //tca_select(id);
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
    uint16_t x = aaReal.x;
    uint16_t y = aaReal.y;
    uint16_t z = aaReal.z;

    // Serial.println("here they are:");
    //
    float xx = aaReal.x;
    char buf[100];
    // sprintf(buf,"%d;%d;%d", x,y,z);
    sprintf(buf,"%.2f;%d;%d", xx,y,z);
    format_reading(reading_data_array, x, y, z, 0, 0, 0);
    // Serial.println(x);
    // Serial.println(y);
    // Serial.println(z);
    Serial.println(buf);
}

class someCallback : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* pCharacteristic) {
    Serial.println("Characteristic was read");
  }
};

//class serverCallback : public BLEServerCallbacks {
//void onConnect(BLEServer* pServer){
    
//}
//};

void setup() {
  //byte data[20] = {0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42};
  byte data[40] = {0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42, 0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42};
  //uint16_t data = 0x42 << 8 | 0x42;
  //  pinMode(LED_BLUE, OUTPUT);
  // pinMode(LED_GREEN, OUTPUT);

  // digitalWrite(LED_BLUE, HIGH);
  Serial.begin(115200);

  // ------SETUP SD------------
  // setup_SD();

  // Serial.println("Initialising i2c");
  Wire.begin(SDA, SCL); // Setup i2c channel on defined pins

  setup_mpu(sense1, 0);
  BLEDevice::init("LapCal Test");
  //BLEServer* pServer = BLEDevice::createServer();
  pServer = BLEDevice::createServer();
  //BLEService* pService = pServer->createService(SERVICE_UUID);
  pService = pServer->createService(SERVICE_UUID);
  //BLECharacteristic* pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->setCallbacks(new someCallback());
  pCharacteristic->setValue(data, 40);

  pService->start();

  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}


void loop() {
  sense_readings(sense1);
  //delay(200);
  pCharacteristic->setValue(reading_data_array, 12);
  pCharacteristic->notify();
  delay(10);
}
