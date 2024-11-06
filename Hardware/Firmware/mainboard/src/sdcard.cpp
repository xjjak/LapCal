#include <Arduino.h>

// MicroSD card libraries
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include "config.h"
#include "timestamp.h"

bool sd_card_present = false;

SPIClass spi = SPIClass(VSPI);
File dataFile;

// char timestamp[60];

String filename;

void setup_sdcard(char *timestamp) {

  Serial.println("Setting up SD Card");
  spi.begin(SCK, MISO, MOSI, CS);
  if (!SD.begin(CS, spi, 80000000)) {
    Serial.println("Card Mount Failed");
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached, using serial connection");
    digitalWrite(LED_BLUE, LOW);

  } else {
    sd_card_present = true;
    Serial.println("Done setting up SD Card");
    // gen_file_name(filename, tmie);
    filename = String("/data-at-" + String(timestamp) + ".txt");
    dataFile = SD.open(filename, FILE_APPEND);
  }
}

void write_values(char *data) {
  // Serial.println("Writing values...");
  // Serial.println(data);
  if (sd_card_present) {
    dataFile.println(data);
    dataFile.flush();
  } else {
    Serial.println(data);
  }
}
