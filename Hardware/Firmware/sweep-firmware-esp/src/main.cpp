#include <Arduino.h>

#define LED LED_BUILTIN

#define top_pin D3
#define mid_pin D2
#define bot_pin D1

void blink(){
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
}


byte read_column(uint8_t const &top, uint8_t const &middle, uint8_t const &bottom){
    byte out = 0;
    out = out | (!digitalRead(top) << 2);
    out = out | (!digitalRead(middle) << 1);
    out = out | (!digitalRead(bottom) << 0);
    return out;
}

void setup(){
    Serial.begin(9600);
    Serial.swap();
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    pinMode(top_pin, INPUT_PULLUP);
    pinMode(mid_pin, INPUT_PULLUP);
    pinMode(bot_pin, INPUT_PULLUP);
}

bool send_flag = 0;

void loop(){
    while (Serial.available()){
        Serial.read();
        send_flag = 1;
    }
    delayMicroseconds(100);
    if (send_flag){
        Serial.write(read_column(top_pin, mid_pin, bot_pin));
        send_flag = 0;
    }
    delayMicroseconds(100);
}
