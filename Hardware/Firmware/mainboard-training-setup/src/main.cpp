#include <Arduino.h>
 
#include "I2Cdev.h"
#include <MPU6050_6Axis_MotionApps20.h>
#include <Wire.h>

#define LED_GREEN 33
#define LED_BLUE 13

#define SDA 21
#define SCL 22

#define RX_s 25
#define TX_s 26

uint32_t prev_millis;

uint16_t FIFOpacketSize = 42;
uint16_t fifoCount;
uint8_t fifoBuffer[64];
// orientation/motion vars
Quaternion q;   // [w, x, y, z]         quaternion container
VectorInt16 aa; // [x, y, z]            accel sensor measurements
VectorInt16 aaReal; // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld; // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity; // [x, y, z]            gravity vector
float euler[3];      // [psi, theta, phi]    Euler angle container
float ypr[3]; // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

struct offsets {
    int xa, ya, za, xg, yg, zg;
};

const offsets mpu_offsets[] = {
    {-262, -2994, 1711, -174, 214, 46}, // Thumb
    {-6018, 1394, 1385, 66, -86, 35}, // Index
    {-3296, 434, 1879, 377, -178, -6}, // Middle
    {-889, -4586, 1050, 117, -242, 54}, // Ring 
    {2071, -3990, 1582, -42, 119, 58}, // Pinky
    {-2539, -2304, 372, 90, 3, 18}, // Onboard
};

struct reading {
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
};

HardwareSerial sweepSerial(1);

uint8_t sensor_id = 2;

MPU6050* mpu_ptr;

byte sweepState;

char packet[16];

void setup_sensor(MPU6050 *mpu){
    mpu->initialize();
    Serial.println(mpu->testConnection() ? F("MPU6050 connection successful") // Verification...
                                        : F("MPU6050 connection failed"));

    uint8_t devStatus = mpu->dmpInitialize();
    mpu->setXAccelOffset(mpu_offsets[sensor_id].xa);
    mpu->setYAccelOffset(mpu_offsets[sensor_id].ya);
    mpu->setZAccelOffset(mpu_offsets[sensor_id].za);
    mpu->setXGyroOffset(mpu_offsets[sensor_id].xg);
    mpu->setYGyroOffset(mpu_offsets[sensor_id].yg);
    mpu->setZGyroOffset(mpu_offsets[sensor_id].zg);
    if (devStatus == 0) {
        mpu->setDMPEnabled(true);
    } else {
        Serial.println("DMP failed.");
        while(1){delay(10);}
    }
    // FIFOpacketSize = mpu->dmpGetFIFOPacketSize(); // optional
}

void pack_reading(char* packet_buffer, reading* sensor_data, byte* kb_state){
    packet_buffer[0] = *kb_state;
    packet_buffer[1] = sensor_data->ax >> 8 & 0xFF;
    packet_buffer[1 + 1] = sensor_data->ax >> 0 & 0xFF;
    packet_buffer[1 + 2] = sensor_data->ay >> 8 & 0xFF;
    packet_buffer[1 + 3] = sensor_data->ay >> 0 & 0xFF;
    packet_buffer[1 + 4] = sensor_data->az >> 8 & 0xFF;
    packet_buffer[1 + 5] = sensor_data->az >> 0 & 0xFF;
    packet_buffer[1 + 6] = sensor_data->gx >> 8 & 0xFF;
    packet_buffer[1 + 7] = sensor_data->gx >> 0 & 0xFF;
    packet_buffer[1 + 8] = sensor_data->gy >> 8 & 0xFF;
    packet_buffer[1 + 9] = sensor_data->gy >> 0 & 0xFF;
    packet_buffer[1 + 10] = sensor_data->gz >> 8 & 0xFF;
    packet_buffer[1 + 11] = sensor_data->gz >> 0 & 0xFF;
}

void get_kb_state(byte *out){
    sweepSerial.write('A');
    delayMicroseconds(10);
    while (!sweepSerial.available()){}
    // Serial.println("Byte available");
    byte incoming = sweepSerial.read();
    *out = incoming;
    // Serial.println("Byte read");
}

void get_all_data(char* packet_buf, byte* kb_state, MPU6050* mpu){
    mpu->resetFIFO();
    // Serial.println("Reset worked");
    get_kb_state(kb_state);
    // Serial.println("sweep worked");
    fifoCount = mpu->getFIFOCount();
    // prev_millis = millis();
    while (fifoCount<FIFOpacketSize){fifoCount = mpu->getFIFOCount();} // wait for full packet
    mpu->getFIFOBytes(fifoBuffer, FIFOpacketSize);
    mpu->dmpGetQuaternion(&q, fifoBuffer);
    mpu->dmpGetAccel(&aa, fifoBuffer);
    mpu->dmpGetGravity(&gravity, &q);
    mpu->dmpGetLinearAccel(&aaReal, &aa, &gravity);
    mpu->dmpGetYawPitchRoll(ypr, &q, &gravity);
    // Serial.println("dmp worked");
    reading cur_reading;
    cur_reading.ax = aaReal.x * -1;
    cur_reading.ay = aaReal.y * -1;
    cur_reading.gy = (int)(ypr[1] * -1 * 100);
    cur_reading.gz = (int)(ypr[2] * -1 * 100);
    cur_reading.gx = (int)(ypr[0] * 100);
    cur_reading.az = aaReal.z;
    // Serial.println("reading worked");
    pack_reading(packet_buf, &cur_reading, kb_state);
    // Serial.println("packing worked");


}

void setup(){
    // Start Serial
    Serial.begin(115200);

    // Setup I2C
    Wire.begin(SDA, SCL);
    Wire.setClock(400000);
    
    // Select Sensor
    Wire.beginTransmission(0x70); // tca_select address
    Wire.write(1 << sensor_id);         // send byte to select bus
    Wire.endTransmission();

    // Verify that sensor is present
    Wire.beginTransmission(0x68);
    byte error;
    error = Wire.endTransmission();
    if (error != 0) {
        Serial.println("Sensor isn't connected!");
        while (1){delay(100);}
    }

    mpu_ptr = new MPU6050();

    sweepSerial.begin(9600, SERIAL_8N1, RX_s, TX_s);
    setup_sensor(mpu_ptr);
    packet[13] = '\n';
    packet[14] = 4;
    packet[15] = 4;
}

void loop(){
    // prev_millis = millis();
    get_all_data(packet, &sweepState, mpu_ptr);
    Serial.write(packet, 16);
    Serial.flush();
    // Serial.print("Millis taken: "); Serial.println(millis() - prev_millis);

}
