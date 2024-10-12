#include <Arduino.h>


#include "config.h"

#include "battery.h"
#include "timestamp.h"
#include "sdcard.h"
#include "touch.h"
#include "sensors.h"

#if BLE
#include "ble-utils.h"

BLECharacteristic *pCharacteristic;
#endif

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
//uint64_t timestamp_num;
uint64_t milli_timestamp;

TaskHandle_t TaskFifoReset;

void print_headerline(){
}


void setup(){
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(SD_DETECT, INPUT);

    digitalWrite(LED_BLUE, HIGH);
    Serial.begin(115200);

    // char headerline[500];
    // headerline[0] = (char)0;

    Serial.println("Initialising i2c");
    setup_i2c(SDA, SCL);
    
    // ------SETUP SENSORS-------
    Serial.println("Setting up Sensors");
    setup_sensors();
    
    // ------SETUP SD------------
    //gen_timestamp(unix_timestamp, &timestamp_num);
    //Serial.println(unix_timestamp);
    // sprintf(headerline, "%d", unix_timestamp);
    //strcat(headerline, unix_timestamp);
    //setup_sdcard(unix_timestamp);
    //write_values(headerline);
    
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, HIGH);

    // ------GET TIMESTAMP-------
    milli_timestamp = get_milli_timestamp();
    //milli_timestamp = 1010101010;
    Serial.println(milli_timestamp);
    
    // ------Parallelization setup------
    if (MULT_CORE){
        xTaskCreatePinnedToCore(task_fifo_reset, "fifo_resets", 10000, NULL, 1, &TaskFifoReset, 0);
        Serial.println("Parallelization enabled.");
    }

    #if BLE
    pCharacteristic = setup_ble();
    #endif
    timer_battery_check.start();



    // ---- TIME SINGLE CYCLE ----
    //
    // uint32_t prev_millis = millis();
    // get_all_readings(all_readings);
    // format_readings(all_readings, all_readings_charbuf);
    // Serial.print(".");
    // ble_transmit_values(pCharacteristic, all_readings, milli_timestamp);
    // write_values(all_readings_charbuf);
    // Serial.print("Millis taken: "); Serial.println(millis() - prev_millis);
     
}


void loop(){
    #if BLE
      if (is_ble_connected()){
        get_all_readings(all_readings);
        ble_transmit_values(pCharacteristic, all_readings, milli_timestamp);
      }
    #else
      get_all_readings(all_readings);
      format_readings(all_readings, all_readings_charbuf, milli_timestamp);
      Serial.println(all_readings_charbuf);
    #endif
   
    // Serial.println("Checking bat_stat");
    if (check_bat_flag){
    // Serial.println("Checking bat...");
        check_battery();
        check_bat_flag = false;
    }

    // Serial.println("detecting touch");
    // detect_touch(TOUCH_PIN);

    // Serial.println("Loop done.");
    timer_battery_check.update();

}
