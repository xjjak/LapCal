#include <Arduino.h>

// MPU6050 libraries
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// MicroSD card libraries
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Libraries to get time from NTP Server
#include <WiFi.h>
#include <ezTime.h>

// Tickers
#include <TickTwo.h>

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

#define SENSOR_COUNT 5



String output;

SPIClass spi = SPIClass(VSPI);
File data_File;

Adafruit_MPU6050 sense1;
Adafruit_MPU6050 sense2;
Adafruit_MPU6050 sense3;
Adafruit_MPU6050 sense4;
Adafruit_MPU6050 sense5;

Adafruit_MPU6050 Sensors[] = {sense1, sense2, sense3, sense4, sense5};
bool sensor_presence[] = {false, false, false, false, false};

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

void setup_sensor(Adafruit_MPU6050& mpu_unit, int id) {
  Serial.print("Setting up Sensor: ");Serial.println(id);
  tca_select(id);
  if (!mpu_unit.begin()) {
    Serial.println("mpu_unit failed.");
    return;
  }
  sensor_presence[id] = true;
  Serial.println("mpu_unit found.");
  mpu_unit.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu_unit.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu_unit.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

String sense_readings(Adafruit_MPU6050& mpu_unit) {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu_unit.getEvent(&a, &g, &temp);

  String ax = String(a.acceleration.x);
  String ay = String(a.acceleration.y);
  String az = String(a.acceleration.z);
  String gx = String(g.gyro.x);
  String gy = String(g.gyro.y);
  String gz = String(g.gyro.z);

  String output = String(ax+";"+ay+";"+az+";"+gx+";"+gx+";"+gx);

  return output;
}

// char[] format_readings(ax, ay, az, gx, gy, gz) {
//     int[] vals = {ax*1000, ay*1000, az*1000, gx*1000, gy*1000, gz*1000};
// }

String get_all_readings() {
  String all_readings[5];
  for (int i=0;i<SENSOR_COUNT;i++) {
    if (sensor_presence[i]) {
      tca_select(i);
      all_readings[i] = sense_readings(Sensors[i]);
    } else {
      all_readings[i] = "none";
    }
  }
    
  return String(all_readings[0] + ":" + all_readings[1] + ":" + all_readings[2] + ":" + all_readings[3] + ":" + all_readings[4]);
}

void setup_SD(){
  Serial.println("Setting up SD Card");
  spi.begin(SCK, MISO, MOSI, CS);
  if(!SD.begin(CS, spi, 80000000)){
    Serial.println("Card Mount Failed");
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    digitalWrite(LED_BLUE, LOW);
    while(true) {

    }
  }
  Serial.println("Done setting up SD Card");
  
  data_File = SD.open(gen_file_name(), FILE_APPEND);
}

void write_values(String data) {
  data_File.println(data);
  data_File.flush();
}

void detect_touch(uint8_t pin){
  touch_val = touchRead(pin);

  if (touch_val > THRESH_NO_TOUCH and cur_touched_state == true) {
    cur_touched_state = false;
    // Serial.println("nt");
    write_values("nt");
  } else if (touch_val < THRESH_TOUCH and cur_touched_state == false) {
    cur_touched_state = true;
    write_values("t");
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
  // setup_SD();

  Serial.println("Initialising i2c");
  Wire.begin(SDA, SCL); // Setup i2c channel on defined pins

  // ------SETUP SENSORS-------
  Serial.println("Setting up Sensors");
  for (int i=0;i<SENSOR_COUNT;i++) {
    setup_sensor(Sensors[i], i);
  } 

  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, HIGH);

  timer_battery_check.start();
  
  // uint32_t prev_millis = millis();
  // write_values(get_all_readings());
  // Serial.print("Millis taken: "); Serial.println(millis() - prev_millis);
}

void loop(){
  write_values(get_all_readings());
  // Serial.println(get_all_readings());
  // Serial.println(battery_read());
  if (check_bat_flag){
    Serial.println("Checking bat...");
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


  delay(300);
  timer_battery_check.update();
      
  detect_touch(TOUCH_PIN);
}
