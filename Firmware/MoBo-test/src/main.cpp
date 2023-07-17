#include <Arduino.h>

#define LED_BUILTIN 33


void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    // Serial.begin(115200);
}

void loop() {
    Serial.println("Hello World!");
    delay(100);
    // digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}
