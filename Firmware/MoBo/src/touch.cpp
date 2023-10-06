#include <Arduino.h>

#include "config.h"
#include "sdcard.h"

bool cur_touched_state = false;
uint16_t touch_val;


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
