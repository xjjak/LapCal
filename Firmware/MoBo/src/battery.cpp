#include "Arduino.h"
#include "config.h"
// int THRESH_LOW_BAT = 1800;


void blink(int pin){
  digitalWrite(pin,HIGH);
  delay(300);
  digitalWrite(pin,LOW);
  delay(300);
  digitalWrite(pin,HIGH);
  delay(300);
  digitalWrite(pin,LOW);
}

int battery_read() {
  int voltage = analogRead(VBAT_SENSE);
  // Serial.println(voltage);
  return voltage;
}

bool battery_connected(){
  return !(battery_read() < 5);
}

void check_battery(){
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
}
