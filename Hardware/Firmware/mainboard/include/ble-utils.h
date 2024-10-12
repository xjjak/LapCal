#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
int is_ble_connected();
BLECharacteristic* setup_ble();
void ble_transmit_values(BLECharacteristic* characteristic, reading* readings, uint64_t timestamp);
