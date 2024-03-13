#include <Arduino.h>


#include "config.h"

#include "battery.h"
#include "timestamp.h"
#include "sdcard.h"
#include "touch.h"
#include "sensors.h"


// #if BLE_SERIAL
//
// #include <BLEDevice.h>
// #include <BleSerial.h>
// BleSerial ble;
//
// #endif

#include <BLEDevice.h>
#include <BleSerial.h>
BleSerial ble;

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

char unix_timestamp[60];

TaskHandle_t TaskFifoReset;

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

    Serial.println("Initialising i2c");
    setup_i2c(SDA, SCL);
    
    // ------SETUP SENSORS-------
    Serial.println("Setting up Sensors");

    setup_sensors();
    
    // ------SETUP SD------------
    gen_timestamp(unix_timestamp);
    Serial.println(unix_timestamp);
    // sprintf(headerline, "%d", unix_timestamp);
    strcat(headerline, unix_timestamp);
    setup_sdcard(unix_timestamp);
    
    write_values(headerline);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, HIGH);
    
    // ------Parallelization setup------
    if (MULT_CORE){
        xTaskCreatePinnedToCore(task_fifo_reset, "fifo_resets", 10000, NULL, 1, &TaskFifoReset, 0);
        Serial.println("Parallelization enabled.");
    }
    
    timer_battery_check.start();

    if (BLE_SERIAL){
// #if BLE_SERIAL
        ble.begin("Lapcal left");
        ble.println("Hello World!");
        Serial.println("BLESerial started.");
// #endif
    }

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


    if (BLE_SERIAL){
        ble.println(all_readings_charbuf);
    }
    else {
        write_values(all_readings_charbuf);
    }
   
// #if BLE_SERIAL
//     ble.println(all_readings_charbuf);
// #else
//     write_values(all_readings_charbuf);
// #endif


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
