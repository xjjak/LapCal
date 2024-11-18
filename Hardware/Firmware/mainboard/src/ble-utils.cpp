#include <Arduino.h>

#include "config.h"
#include "ble-utils.h"

#define service_uuid "f5aea478-9ec3-4bcf-af20-7f75e7c68c9d"
#define characteristic_uuid "68bf07fb-d00b-4c80-a796-f8be82b5dea7"

BLEServer *pBleServer;
BLEService *pBleService;
BLECharacteristic *pBleCharacteristic;
BLEAdvertising *pBleAdvertising;

// 8*8 (timestamp) + ((2*8)*6)*6 = 80
#define packet_length 80
byte all_data_array[packet_length];

int ble_is_connected = 0;

int is_ble_connected() { return ble_is_connected; }

void pack_reading(reading *readings, byte *all_data_array, uint64_t timestamp) {
  for (int i = 0; i < 8; i++) {
    all_data_array[7 - i] = timestamp >> 8 * i & 0xFF;
  }
  for (int s = 0; s <= 6; s++) {
    all_data_array[8 + (s * 12)] = readings[s].ax >> 8 & 0xFF;
    all_data_array[8 + (s * 12) + 1] = readings[s].ax >> 0 & 0xFF;
    all_data_array[8 + (s * 12) + 2] = readings[s].ay >> 8 & 0xFF;
    all_data_array[8 + (s * 12) + 3] = readings[s].ay >> 0 & 0xFF;
    all_data_array[8 + (s * 12) + 4] = readings[s].az >> 8 & 0xFF;
    all_data_array[8 + (s * 12) + 5] = readings[s].az >> 0 & 0xFF;
    all_data_array[8 + (s * 12) + 6] = readings[s].gx >> 8 & 0xFF;
    all_data_array[8 + (s * 12) + 7] = readings[s].gx >> 0 & 0xFF;
    all_data_array[8 + (s * 12) + 8] = readings[s].gy >> 8 & 0xFF;
    all_data_array[8 + (s * 12) + 9] = readings[s].gy >> 0 & 0xFF;
    all_data_array[8 + (s * 12) + 10] = readings[s].gz >> 8 & 0xFF;
    all_data_array[8 + (s * 12) + 11] = readings[s].gz >> 0 & 0xFF;
  }
}

void ble_advertise(BLEServer *pBleServer) {
  Serial.print("Starting advertisement...");
  pBleAdvertising = pBleServer->getAdvertising();
  pBleAdvertising->start();
  Serial.println(" successful.");
}

class BleEventCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) {
    ble_is_connected = 1;
    Serial.println("Connection established.");
    digitalWrite(LED_BLUE, HIGH);
    // pBleAdvertising->stop();
  }
  void onDisconnect(BLEServer *server) {
    ble_is_connected = 0;
    Serial.println("Connection lost");
    digitalWrite(LED_BLUE, LOW);
    ble_advertise(server);
  }
};

// class BleCharacteristicCallbacks : public BLECharacteristicCallbacks {
//   void onRead(BLECharacteristic *pCharacteristic){
//   }
// };

BLECharacteristic *setup_ble() {
  ble_is_connected = 0;
  Serial.println("Setting up BLE...");
#if RIGHT_HAND
  BLEDevice::init("lapcal right");
#else
  BLEDevice::init("lapcal left");
#endif
  pBleServer = BLEDevice::createServer();
  pBleService = pBleServer->createService(service_uuid);
  pBleCharacteristic = pBleService->createCharacteristic(
      characteristic_uuid,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pBleServer->setCallbacks(new BleEventCallbacks());
  // pBleCharacteristic->setCallbacks(new BleCharacteristicCallbacks());
  pBleCharacteristic->setValue({0x42});
  pBleService->start();

  ble_advertise(pBleServer);
  return pBleCharacteristic;
}

void ble_transmit_values(BLECharacteristic *characteristic, reading *readings,
                         uint64_t timestamp) {
  pack_reading(readings, all_data_array, timestamp + millis());
  // Serial.println(all_data_array);
  characteristic->setValue(all_data_array, 76);
  characteristic->notify();
}
