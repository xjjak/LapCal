#include <Arduino.h>


#include "config.h"

#include "battery.h"
#include "timestamp.h"
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

unsigned long prev_micros2;

char all_readings_charbuf[SENSOR_COUNT*201];

char unix_timestamp[60];

void print_headerline(){
}


void setup(){
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(SD_DETECT, INPUT);

    digitalWrite(LED_BLUE, HIGH);
    Serial.begin(115200);

    char headerline[500];
    headerline[0] = (char)0;

    // ------SETUP SD------------
    gen_timestamp(unix_timestamp);
    Serial.println(unix_timestamp);
    // sprintf(headerline, "%d", unix_timestamp);
    strcat(headerline, unix_timestamp);
    setup_sdcard(unix_timestamp);

    Serial.println("Initialising i2c");
    setup_i2c(SDA, SCL);

    // ------SETUP SENSORS-------
    Serial.println("Setting up Sensors");
    char unit_buffer[30];
    for (int i=0;i<SENSOR_COUNT;i++) {
        if (setup_sensor(i)) {
            Serial.println("--------WARNING: A SENSOR FAILED-------");
        }
        sprintf(unit_buffer, ":g%dx;g%dy;g%dz;a%dy;a%dp;a%dr", i,i,i,i,i,i);
        strcat(headerline, unit_buffer);
    } 

    write_values(headerline);
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
    prev_micros2 = micros();

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

    Serial.printf("after writing: %d \n", micros() - prev_micros2);
          
}
