/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-microsd-card-arduino/
  
  This sketch can be found at: Examples > SD(esp32) > SD_Test
*/

#include "SD.h"
#include "SPI.h"

#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 5

SPIClass spi = SPIClass(VSPI);

File dataFile;

struct datastore {
    uint16_t adc1;
    float voltage;
    float current;
};

void setup() {
    spi.begin(SCK, MISO, MOSI, CS);
    Serial.begin(115200);
    Serial.print("Hello");
    if(!SD.begin(CS, spi, 80000000)){
        Serial.println("Card Mount Failed");
        return;
    }

    Serial.println("card initialized.");
    dataFile = SD.open("/datalog.dat", FILE_READ);
}

void loop() {
    if (dataFile.available()) {
        struct datastore myData;
        dataFile.read((uint8_t *)&myData, sizeof(myData));
        Serial.println(myData.adc1);
        Serial.print(myData.voltage, 4);
        Serial.print(" ");
        Serial.println(myData.current, 4);
        delay(500);
    }
}
