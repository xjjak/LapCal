#include <Arduino.h>

/* Left hand:
    [ "E6", "F7", "F6", "F5", "F4"],
    ["B1", "B3", "B2", "B6", "D3"],
    ["D1", "D0", "D4", "C6", "D7"],
    ["B4", "B5", null, null, null] 
    Pin numbers:
    [ "7", "A0", "A1", "A2", "A3"],
    ["15", "14", "16", "A10", "3->D1?"],
    ["TXO", "RXI", "4/A6", "5", "6/A7"],
    ["A8", "A9", null, null, null] 
   Right hand:
    ["F4", "F5", "F6", "F7", "E6"],
    ["D3", "B6", "B2", "B3", "B1"],
    ["D7", "C6", "D4", "D0", "D1"],
    ["B5", "B4", null, null, null]
   Pins:
    ["F4", "F5", "F6", "F7", "E6"],
    ["D3", "B6", "B2", "B3", "B1"],
    ["D7", "C6", "D4", "D0", "D1"],
    ["9/A9", "8/A8", null, null, null]
    */

#define SW_3 A1
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

uint8_t read_column(uint8_t const &top, uint8_t const &middle, uint8_t const bottom){
    uint8_t out = 0;
    out = out | (!digitalRead(top) << 2);
    out = out | (!digitalRead(middle) << 1);
    out = out | (!digitalRead(bottom) << 0);
    return out;
}


void loop() {
    Serial.println(read_column(SW_3, SW_8, SW_13));
    delay(10);
}
