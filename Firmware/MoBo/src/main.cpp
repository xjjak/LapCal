#include <Arduino.h>

// MPU6050 libraries
#include <MPU6050_6Axis_MotionApps20.h>
#include <Wire.h>

#include "config.h"

// MicroSD card libraries
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Libraries to get time from NTP Server
#include <WiFi.h>
#include <ezTime.h>

// Tickers
#include <TickTwo.h>

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


bool sd_card_present = false;

bool check_bat_flag = false;
void set_bat_flag() {
  check_bat_flag = true;
}

TickTwo timer_battery_check(set_bat_flag, 300000, 0, MILLIS);


const char* ssid = "schueler";
const char* password = "lundlundlund";

// NTP Server to request epoch time
const char* ntpServer = "de.pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime; 


// Touch pin value variables
// uint16_t prev_touch_val;
bool cur_touched_state = false;
uint16_t touch_val;

// touch thresholds
uint16_t THRESH_TOUCH = 15;
uint16_t THRESH_NO_TOUCH = 30;

int THRESH_LOW_BAT = 1800;




String output;

SPIClass spi = SPIClass(VSPI);
File dataFile;

//------ MPU DECLARATIONS ------

// MPU6050 sense1;
// MPU6050 sense2;
// MPU6050 sense3;
// MPU6050 sense4;
// MPU6050 sense5;
//
// MPU6050 mpu = Sensors[4];

// MPU6050* Sensors[] = {sense1, sense2, sense3, sense4, sense5};
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

struct reading {
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
};

//---------------------
char all_readings_char[SENSOR_COUNT*201];


void blink(int pin){
  digitalWrite(pin,HIGH);
  delay(300);
  digitalWrite(pin,LOW);
  delay(300);
  digitalWrite(pin,HIGH);
  delay(300);
  digitalWrite(pin,LOW);
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}
//
// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

String gen_file_name() {
  initWiFi();
  waitForSync();
  Timezone Germany;
  Germany.setLocation("Europe/Berlin");
  // time_t epoch_now = now();
  String time_now = String(now());
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  return String("/data-at-" + time_now + ".txt");
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

void setup_sensor(int id) {
    Serial.print("Setting up Sensor: ");Serial.println(id);


    tca_select(id);

    if (!mpu_present()){
        Serial.println("Nothing there.");
        sensor_presence[id] = false;
        return;
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

    // Serial.println("here they are:");
    //
    // char buf[200];
    // sprintf(buf,"%.2f;%.2f;%.2f;%.2f;%.2f;%.2f", output.ax,output.ay,output.az,output.gx,output.gy,output.gz);
    // Serial.println(buf);
    // Serial.println(output.gx);
    // return buf;
    // Serial.println("Returning values...");
    return output;
}

void get_all_readings(char* output_buf) {
    // char output_buf[1005];
    output_buf[0] = (char)0;
    char return_buf[200];
    reading values_read;
    for (int i=0;i<SENSOR_COUNT;i++) {
        if (sensor_presence[i]) {
            tca_select(i);
            values_read = sense_readings(Sensors[i]);
            sprintf(return_buf,"%.2f;%.2f;%.2f;%.2f;%.2f;%.2f:", values_read.ax,values_read.ay,values_read.az,values_read.gx,values_read.gy,values_read.gz);
            strcat(output_buf, return_buf);
            // Serial.println("Got some values");
        } else {
            strcat(output_buf, "-:");
        }
    }
    // output_string = String(all_readings[0] + ":" + all_readings[1] + ":" + all_readings[2] + ":" + all_readings[3] + ":" + all_readings[4]);
    // Serial.println(output_string);
    // return output_string;
    // return String(all_readings[0] + ":" + all_readings[1] + ":" + all_readings[2] + ":" + all_readings[3] + ":" + all_readings[4]);
    // Serial.println(output_buf);
    // return output_buf;
}

void setup_SD(){
  Serial.println("Setting up SD Card");
  spi.begin(SCK, MISO, MOSI, CS);
  if(!SD.begin(CS, spi, 80000000)){
    Serial.println("Card Mount Failed");
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached, using serial connection");
    digitalWrite(LED_BLUE, LOW);
    // while(true) {
    //
    // }
  } else {
      sd_card_present = true;
      Serial.println("Done setting up SD Card");
      
      dataFile = SD.open(gen_file_name(), FILE_APPEND);
  }
}

void write_values(char* data) {
  // Serial.println("Writing values...");
  // Serial.println(data);
  if (sd_card_present){
      dataFile.println(data);
      dataFile.flush();
  } else {
      Serial.println(data);
  }
}

void detect_touch(uint8_t pin){
  touch_val = touchRead(pin);

  if (touch_val > THRESH_NO_TOUCH and cur_touched_state == true) {
    cur_touched_state = false;
    // Serial.print("nt");
    char no_t[] = "nt";
    write_values(no_t);
  } else if (touch_val < THRESH_TOUCH and cur_touched_state == false) {
    cur_touched_state = true;
    char to[] = "t";
    write_values(to);
    // Serial.println("t");
  }
}

int battery_read() {
  int voltage = analogRead(VBAT_SENSE);
  // Serial.println(voltage);
  return voltage;
}

bool battery_connected(){
  return !(battery_read() < 5);
}


void setup(){
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(SD_DETECT, INPUT);

  digitalWrite(LED_BLUE, HIGH);
  Serial.begin(115200);

  // ------SETUP SD------------
  setup_SD();

  Serial.println("Initialising i2c");
  Wire.begin(SDA, SCL); // Setup i2c channel on defined pins

  // ------SETUP SENSORS-------
  Serial.println("Setting up Sensors");
  for (int i=0;i<SENSOR_COUNT;i++) {
    setup_sensor(i);
  } 

  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, HIGH);

  timer_battery_check.start();
  
  // uint32_t prev_millis = millis();
  // write_values(get_all_readings());
  // Serial.print("Millis taken: "); Serial.println(millis() - prev_millis);
}

void loop(){
  // Serial.println("Sensing.");
  get_all_readings(all_readings_char);
  Serial.print(".");
  write_values(all_readings_char);
  // Serial.println("Checking bat_stat");
  if (check_bat_flag){
    // Serial.println("Checking bat...");
    if (battery_connected()) {
      if (battery_read() < THRESH_LOW_BAT) {
        esp_sleep_enable_timer_wakeup(86400000000);
        Serial.println("Threshhold reached. Idling");
        blink(LED_GREEN);
        delay(1000);
        Serial.flush(); 
        esp_deep_sleep_start();
      }
    }
    check_bat_flag = false;
  }

  // Serial.println("detecting touch");
  detect_touch(TOUCH_PIN);

  // Serial.println("Loop done.");
  // delay(1);
  timer_battery_check.update();
      
}
