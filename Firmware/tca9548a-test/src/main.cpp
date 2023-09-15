#include <Arduino.h>

#define LED_BUILTIN 13
#define SENSOR_COUNT 1
#define SDA 21
#define SCL 22

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 sense1;
Adafruit_MPU6050 sense2;

// Select I2C BUS
void tca_select(uint8_t bus){
  Wire.beginTransmission(0x70);  // tca_select address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

void setup_mpu(Adafruit_MPU6050 mpu_unit, int id)   {
  if (!mpu_unit.begin(0x68, &Wire, id)) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu_unit.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu_unit.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu_unit.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

void sense_readings(Adafruit_MPU6050& mpu_unit) {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu_unit.getEvent(&a, &g, &temp);

  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

  Serial.println("");
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  Wire.begin(SDA, SCL); // Setup i2c channel on defined pins

  // for (int i=0;i<SENSOR_COUNT;i++) {
  //     tca_select(i);

  // tca_select(0);
  // setup_mpu(sense1, 0);
  // tca_select(1);
  // setup_mpu(sense2, 1);
  //
  tca_select(5);
  if (!sense1.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  sense1.setAccelerometerRange(MPU6050_RANGE_8_G);
  sense1.setGyroRange(MPU6050_RANGE_500_DEG);
  sense1.setFilterBandwidth(MPU6050_BAND_5_HZ);

  // tca_select(1);
  // if (!sense2.begin()) {
  //   Serial.println("Failed to find MPU6050 chip");
  //   while (1) {
  //     delay(10);
  //   }
  // }
  // Serial.println("MPU6050 Found!");
  //
  // sense2.setAccelerometerRange(MPU6050_RANGE_8_G);
  // sense2.setGyroRange(MPU6050_RANGE_500_DEG);
  // sense2.setFilterBandwidth(MPU6050_BAND_5_HZ);


  // Try to initialize!
  Serial.println("");
  delay(100);
}

void loop() {
    // delay(10);
    // tca_select(1);
    // sense_readings(sense2);
    // delay(1000);

  tca_select(5);
  sense_readings(sense1);
  // sensors_event_t a, g, temp;
  // sense1.getEvent(&a, &g, &temp);
  // //
  // // // /* Print out the values */
  // Serial.print("SENSE 1:  ");
  // Serial.print("Acceleration X: ");
  // Serial.print(a.acceleration.x);
  // Serial.print(";");
  // Serial.print(a.acceleration.y);
  // Serial.print(";");
  // Serial.print(a.acceleration.z);
  // Serial.print(";");
  //
  // Serial.print(g.gyro.x);
  // Serial.print(";");
  // Serial.print(g.gyro.y);
  // Serial.print(";");
  // Serial.print(g.gyro.z);
  // Serial.println("");

  // tca_select(1);
  // sensors_event_t a2, g2, temp2;
  // sense2.getEvent(&a2, &g, &temp2);
  // //
  // // // /* Print out the values */
  // Serial.print("SENSE 2:  ");
  // Serial.print("Acceleration X: ");
  // Serial.print(a2.acceleration.x);
  // Serial.print(";");
  // Serial.print(a2.acceleration.y);
  // Serial.print(";");
  // Serial.print(a2.acceleration.z);
  // Serial.print(";");
  //
  // Serial.print(g2.gyro.x);
  // Serial.print(";");
  // Serial.print(g2.gyro.y);
  // Serial.print(";");
  // Serial.print(g2.gyro.z);
  // Serial.println("");
  delay(1000);
}
