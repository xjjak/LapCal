#include <Arduino.h>


#include "config.h"

#include "battery.h"
#include "sdcard.h"
#include "touch.h"
#include "sensors.h"

// ------ TICKERS ------
#include <TickTwo.h>
bool check_bat_flag = false;
void set_bat_flag() {
  check_bat_flag = true;
}

TickTwo timer_battery_check(set_bat_flag, 300000, 0, MILLIS);

// ---------------------

reading all_readings[SENSOR_COUNT];

char all_readings_charbuf[SENSOR_COUNT*201];


void setup(){
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(SD_DETECT, INPUT);

    digitalWrite(LED_BLUE, HIGH);
    Serial.begin(115200);

    // ------SETUP SD------------
    setup_sdcard();

    Serial.println("Initialising i2c");
    setup_i2c(SDA, SCL);

    // ------SETUP SENSORS-------
    Serial.println("Setting up Sensors");
    for (int i=0;i<SENSOR_COUNT;i++) {
    setup_sensor(i);
    } 

    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, HIGH);

    timer_battery_check.start();
  
    // ---- TIME SINGLE CYCLE ----
    //
    // uint32_t prev_millis = millis();
    // get_all_readings(all_readings);
    // format_readings(all_readings, all_readings_charbuf);
    // Serial.print(".");
    // write_values(all_readings_charbuf);
    // Serial.print("Millis taken: "); Serial.println(millis() - prev_millis);
}


void loop(){
    // Serial.println("Sensing.");
    get_all_readings(all_readings);
    format_readings(all_readings, all_readings_charbuf);
    write_values(all_readings_charbuf);
    // Serial.println("Checking bat_stat");
    if (check_bat_flag){
    // Serial.println("Checking bat...");
        check_battery();
        check_bat_flag = false;
    }

    // Serial.println("detecting touch");
    detect_touch(TOUCH_PIN);

    // Serial.println("Loop done.");
    timer_battery_check.update();
          
}
