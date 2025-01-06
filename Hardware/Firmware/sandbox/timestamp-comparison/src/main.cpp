#include <Arduino.h>

#include "timestamp.h"

#define LED_GREEN 33
#define LED_BLUE 13

#ifndef BLE
#define BLE 0
#endif 

#if BLE
#include "ble-utils.h"

BLECharacteristic *pCharacteristic;
#endif


uint64_t milli_timestamp;

void setup() {
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_BLUE, HIGH);
  Serial.begin(115200);



  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, HIGH);

  // ------GET TIMESTAMP-------
  milli_timestamp = get_milli_timestamp();
  // milli_timestamp = 1010101010;
  Serial.println(milli_timestamp);


#if BLE
  pCharacteristic = setup_ble();
#endif

}

void loop() {
#if BLE
  if (is_ble_connected()) {
    get_all_readings(all_readings);
    ble_transmit_values(pCharacteristic, all_readings, milli_timestamp);
  }
#else
  Serial.println(milli_timestamp + millis());
  delay(10);
#endif
}
