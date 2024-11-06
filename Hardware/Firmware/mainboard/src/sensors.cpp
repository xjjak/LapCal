#include <Arduino.h>

#include <MPU6050_6Axis_MotionApps20.h>
#include <Wire.h>

#include "config.h"

#define BIT_0 (1 << 0)

unsigned long prev_micros;
unsigned long reading_start_micros;

#define __TIMING(format)                                                       \
  if (TIMING_ENABLED) {                                                        \
    Serial.printf(format, micros() - prev_micros);                             \
    prev_micros = micros();                                                    \
  }
const int TIMING_ENABLED = 0;

MPU6050 Sensors[SENSOR_COUNT];
int sensor_presence[SENSOR_COUNT] = {};

// MPU control/status vars
uint8_t devStatus; // return status after each device operation (0 = success, !0
                   // = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;   // [w, x, y, z]         quaternion container
VectorInt16 aa; // [x, y, z]            accel sensor measurements
VectorInt16
    aaReal; // [x, y, z]            gravity-free accel sensor measurements
VectorInt16
    aaWorld; // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity; // [x, y, z]            gravity vector
float euler[3];      // [psi, theta, phi]    Euler angle container
float
    ypr[3]; // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

bool task_reset_fifos_flag = true;

EventGroupHandle_t FifoResetEventGroup = xEventGroupCreate();
EventBits_t uxBits = xEventGroupSetBits(FifoResetEventGroup, BIT_0);

// char all_readings_char[SENSOR_COUNT*201];

// reading all_readings[SENSOR_COUNT];

void setup_i2c(int sda, int scl) {
  Wire.begin(sda, scl);
  Wire.setClock(400000);
}

// Select I2C BUS
void tca_select(uint8_t bus) {
  Wire.beginTransmission(0x70); // tca_select address
  Wire.write(1 << bus);         // send byte to select bus
  Wire.endTransmission();
}

void select_sensor(int id) {
  // Serial.println(sensor_presence[id]);
  tca_select(sensor_presence[id]);
}

bool mpu_present() {
  Wire.beginTransmission(0x68);
  byte error;
  error = Wire.endTransmission();
  // Serial.println((int) error);
  return (error == 0);
}

int setup_sensor(int id) {
  Serial.print("Setting up Sensor: ");
  Serial.println(id);

  select_sensor(id);

  if (!mpu_present()) {
    Serial.println("Nothing there.");
    sensor_presence[id] = -1;
    return 1;
  }

  MPU6050 mpu = Sensors[id];
  mpu.initialize();

  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful")
                                      : F("MPU6050 connection failed"));

  // verify connection

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();
  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXAccelOffset(
      mpu_offsets[id].xa); // 1688 factory default for my test chip
  mpu.setYAccelOffset(
      mpu_offsets[id].ya); // 1688 factory default for my test chip
  mpu.setZAccelOffset(
      mpu_offsets[id].za); // 1688 factory default for my test chip
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
    // sensor_presence[id] = true;
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
    sensor_presence[id] = -1;
    return 1;
  }

  return 0;
}

void setup_sensors() {
  if (RIGHT_HAND != 1) {
    sensor_presence[0] = 0;
    sensor_presence[1] = 1;
    sensor_presence[2] = 2;
    sensor_presence[3] = 3;
    sensor_presence[4] = 4;
    sensor_presence[5] = 5;
  } else {
    sensor_presence[0] = 4;
    sensor_presence[1] = 3;
    sensor_presence[2] = 2;
    sensor_presence[3] = 1;
    sensor_presence[4] = 0;
    sensor_presence[5] = 5;
  }

  for (int i = 0; i < SENSOR_COUNT; i++) {
    if (setup_sensor(i)) {
      Serial.println("--------WARNING: A SENSOR FAILED-------");
    }
  }
}

void reset_all_bufs() {
  for (int i = 0; i < SENSOR_COUNT; i++) {
    if (sensor_presence[i] != -1) {
      // Serial.print("Resetting sensor: ");
      // Serial.println(i);
      select_sensor(i);
      Sensors[i].resetFIFO();
    }
  }
}

void task_fifo_reset(void *pvParameters) {
  const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;
  for (;;) {
    if (xEventGroupWaitBits(FifoResetEventGroup, BIT_0, pdFALSE, pdTRUE,
                            xTicksToWait) == pdPASS) {
      reset_all_bufs();
      xEventGroupClearBits(FifoResetEventGroup, BIT_0);
    } else {
      Serial.println("Something went wrong waiting for events");
    }
  }
}

void readFifoBuffer(MPU6050 mpu) {

  // Serial.println("Reading fifo buffer...");
  // Clear the buffer so as we can get fresh values
  // The sensor is running a lot faster than our sample period

  __TIMING("after reset: %d \n");

  if (MULT_CORE) {
    uxBits = xEventGroupGetBits(FifoResetEventGroup);
    while ((uxBits & BIT_0) != 0) {
      uxBits = xEventGroupGetBits(FifoResetEventGroup);
    }
  }

  __TIMING("Before get: %d \n");

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();

  __TIMING("after get: %d \n");

  // wait for correct available data length, should be a VERY short wait
  while (fifoCount < packetSize)
    fifoCount = mpu.getFIFOCount();

  __TIMING("before bytes: %d \n");

  // read a packet from FIFO
  mpu.getFIFOBytes(fifoBuffer, packetSize);

  __TIMING("after bytes (t) : %d \n");
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
  // output.gx = ypr[0]* 180/M_PI;
  // output.gy = ypr[1]* 180/M_PI;
  // output.gz = ypr[2]* 180/M_PI;
  if (RIGHT_HAND == 1) {
    output.ax = aaReal.x * -1;
    output.ay = aaReal.y * -1;
    output.gy = (int)(ypr[1] * -1 * 100);
    output.gz = (int)(ypr[2] * -1 * 100);
  } else {
    output.ax = aaReal.x;
    output.ay = aaReal.y;
    output.gy = (int)(ypr[1] * 100);
    output.gz = (int)(ypr[2] * 100);
  }
  output.gx = (int)(ypr[0] * 100);
  output.az = aaReal.z;

  // Serial.println("Returning values...");
  return output;
}

void get_all_readings(reading *output) {
  for (int i = 0; i < SENSOR_COUNT; i++) {
    if (sensor_presence[i] != -1) {
      __TIMING("Before mux: %d \n");
      select_sensor(i);
      __TIMING("After mux: %d \n");
      output[i] = sense_readings(Sensors[i]);
      __TIMING("After readings: %d \n");
      // Serial.println("Got some values");
    }
  }
  if (!MULT_CORE) {
    reset_all_bufs();
  } else {
    xEventGroupSetBits(FifoResetEventGroup, BIT_0);
  }
}

void format_readings(reading *input, char *output_buf, uint64_t timestamp) {
  // char output_buf[1005];
  // output_buf[0] = (char)0;
  sprintf(output_buf, "%lld", (timestamp + micros()));
  char return_buf[200];
  reading cur_reading;
  for (int i = 0; i < SENSOR_COUNT; i++) {
    if (sensor_presence[i] != (-1)) {
      cur_reading = input[i];
      sprintf(return_buf, ":%d;%d;%d;%d;%d;%d", cur_reading.ax, cur_reading.ay,
              cur_reading.az, cur_reading.gx, cur_reading.gy, cur_reading.gz);
      strcat(output_buf, return_buf);
    } else {
      strcat(output_buf, ":-");
    }
  }
}
