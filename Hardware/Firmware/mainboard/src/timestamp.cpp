#include <Arduino.h>

#include <WiFi.h>
#include <ezTime.h>

// #include "config.h"

#ifndef WIFI_SSID
#error Setup wifi.ini and the wifi_flags (these will be added as build_flags in platformio.ini)
#endif

#ifndef WIFI_PASS
#error Setup wifi.ini and the wifi_flags (these will be added as build_flags in platformio.ini)
#endif

#define _STR(x) #x
#define STRINGIFY(x) _STR(x)

const char* ssid = STRINGIFY(WIFI_SSID);
const char* password = STRINGIFY(WIFI_PASS);

// NTP Server to request epoch time
const char* ntpServer = "de.pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime; 

char timestamp_at_boot[60];

String time_now;

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void gen_timestamp(char* start_timestamp){
    char buffer[3];
    initWiFi();
    waitForSync();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    sprintf(start_timestamp, "%d", now());
    sprintf(buffer,"%d",ms());
    strcat(start_timestamp, buffer);
    sprintf(start_timestamp, "%lld", (atoll(start_timestamp)-millis()));
}
