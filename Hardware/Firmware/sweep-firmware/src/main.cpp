#include <Arduino.h>

#include "config.h"

#define SW_3 19
#define SW_8 16
#define SW_13 4


void setup() {

    // Disable JTAG Interface freeing up (enabling interal pull-ups) pins A0-A3
    MCUCR = (1 << JTD); // to actually set the bits the operation has to be repeated in 4 clock cycles
    MCUCR = (1 << JTD);

    Serial.begin(115200);
    delay(10);
    pinMode(SW_3, INPUT_PULLUP);
    pinMode(SW_8, INPUT_PULLUP);
    pinMode(SW_13, INPUT_PULLUP);
}

void loop() {
   Serial.print(digitalRead(SW_3));
   Serial.print(digitalRead(SW_8));
   Serial.println(digitalRead(SW_13));
   delay(10);
}
