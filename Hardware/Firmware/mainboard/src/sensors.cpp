#include <Arduino.h>

#include <MPU6050_6Axis_MotionApps20.h>
#include <Wire.h>

#include "config.h"

unsigned long prev_micros;
unsigned long reading_start_micros;

MPU6050 Sensors[SENSOR_COUNT];
bool sensor_presence[SENSOR_COUNT] = {};

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

// char all_readings_char[SENSOR_COUNT*201];

// reading all_readings[SENSOR_COUNT];

void setup_i2c(int sda, int scl){
    Wire.begin(sda, scl);
    Wire.setClock(400000);
}

// Select I2C BUS
void tca_select(uint8_t bus){
  Wire.beginTransmission(0x70);  // tca_select address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

bool mpu_present(){
    Wire.beginTransmission(0x68);
    byte error;
    error = Wire.endTransmission() ;
    // Serial.println((int) error);
    return (error == 0);
}

int setup_sensor(int id) {
    Serial.print("Setting up Sensor: ");Serial.println(id);


    tca_select(id);

    if (!mpu_present()){
        Serial.println("Nothing there.");
        sensor_presence[id] = false;
        return 1;
    }

    MPU6050 mpu = Sensors[id];
    mpu.initialize();

    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    // verify connection

    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();
    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXAccelOffset(mpu_offsets[id].xa); // 1688 factory default for my test chip
    mpu.setYAccelOffset(mpu_offsets[id].ya); // 1688 factory default for my test chip
    mpu.setZAccelOffset(mpu_offsets[id].za); // 1688 factory default for my test chip
    mpu.setXGyroOffset(mpu_offsets[id].xg);
    mpu.setYGyroOffset(mpu_offsets[id].yg);
    mpu.setZGyroOffset(mpu_offsets[id].zg);


    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
        sensor_presence[id] = true;
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
        return 1;
    }

    return 0;
}

void reset_all_bufs() {
    for (int i=0;i<SENSOR_COUNT;i++) {
        if (sensor_presence[i]) {
            tca_select(i);
            Sensors[i].resetFIFO();
        }
    }
}

void readFifoBuffer(MPU6050 mpu) {

    // Serial.println("Reading fifo buffer...");
    // Clear the buffer so as we can get fresh values
    // The sensor is running a lot faster than our sample period
    Serial.printf("Before reset: %d \n", micros() - prev_micros);
    prev_micros = micros();
    // mpu.resetFIFO();
    Serial.printf("after reset: %d \n", micros() - prev_micros);
    prev_micros = micros();

    Serial.printf("Before get: %d \n", micros() - prev_micros);
    prev_micros = micros();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    Serial.printf("after get: %d \n", micros() - prev_micros);
    prev_micros = micros();
    
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    Serial.printf("before bytes: %d \n", micros() - prev_micros);
    prev_micros = micros();
    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);
    Serial.printf("after bytes: %d \n", micros() - prev_micros);
    prev_micros = micros();
    // Serial.println("Read buffer.");
}

reading sense_readings(MPU6050 mpu) {
    readFifoBuffer(mpu);
    // display real acceleration, adjusted to remove gravity
    // Serial.println("Getting vals");
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    reading output;
    // float x = aaReal.x;
    // float y = aaReal.y;
    // float z = aaReal.z;
    // float yaw = ypr[0]* 180/M_PI;
    // float pitch = ypr[1]* 180/M_PI;
    // float roll = ypr[2]* 180/M_PI;
    output.ax = aaReal.x;
    output.ay = aaReal.y;
    output.az = aaReal.z;
    // output.gx = ypr[0]* 180/M_PI;
    // output.gy = ypr[1]* 180/M_PI;
    // output.gz = ypr[2]* 180/M_PI;
    output.gx = ypr[0];
    output.gy = ypr[1];
    output.gz = ypr[2];

    // Serial.println("Returning values...");
    return output;
}

void get_all_readings(reading* output) {
    reading_start_micros = micros();
    reset_all_bufs();
    for (int i=0;i<SENSOR_COUNT;i++) {
        if (sensor_presence[i]) {
            Serial.printf("Before mux: %d \n", micros() - prev_micros);
            prev_micros = micros();
            tca_select(i);
            Serial.printf("After mux: %d \n", micros() - prev_micros);
            prev_micros = micros();
            output[i] = sense_readings(Sensors[i]);
            Serial.printf("After readings: %d \n", micros() - prev_micros);
            prev_micros = micros();
            // Serial.println("Got some values");
        }     
    }
    Serial.printf("All readings: %d \n", micros() - reading_start_micros);
}

void format_readings(reading* input, char* output_buf) {
    // char output_buf[1005];
    // output_buf[0] = (char)0;
    sprintf(output_buf, "%d:", micros());
    char return_buf[200];
    reading cur_reading;
    for (int i=0;i<SENSOR_COUNT;i++) {
        if (sensor_presence[i]) {
            cur_reading = input[i];
            sprintf(return_buf,"%.2f;%.2f;%.2f;%.2f;%.2f;%.2f:", cur_reading.ax,cur_reading.ay,cur_reading.az,cur_reading.gx,cur_reading.gy,cur_reading.gz);
            strcat(output_buf, return_buf);
        } else {
            strcat(output_buf, "-:");
        }
    }
}
