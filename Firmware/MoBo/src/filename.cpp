#include <Arduino.h>

#include <WiFi.h>
#include <ezTime.h>

#include "config.h"
#include "filename.h"

#ifndef WIFI_SSID
#error Setup wifi.ini and the wifi_flags (these will be added as build_flags in platformio.ini)
#endif

#ifndef WIFI_PASS
#error Setup wifi.ini and the wifi_flags (these will be added as build_flags in platformio.ini)
#endif

#define STR(x) #x

const char* ssid = STR(WIFI_SSID);
const char* password = STR(WIFI_PASS);

// NTP Server to request epoch time
const char* ntpServer = "de.pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime; 

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void gen_file_name(String filename) {
  initWiFi();
  waitForSync();
  Timezone Germany;
  Germany.setLocation("Europe/Berlin");
  // time_t epoch_now = now();
  String time_now = String(now());
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  filename = String("/data-at-" + time_now + ".txt");
}
