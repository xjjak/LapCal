# Mainboard firmware

## Build flags
- `RIGHT_HAND`: Set to `1` to reverse sensor order for right hand, `0` for the left hand. Throws error if not specified
- `MULT_CORE`: Set to `0` to disable parallel buffer resets, `1` to enable. Defaults to `1`.
- `BLE`: Set to `0` to output sensor data to Serial Monitor, `1` enables ble server and data transmission

## BLE Server 
When BLE is enabled the ESP32 acts as an BLE Server providing
- Service with uuid: `f5aea478-9ec3-4bcf-af20-7f75e7c68c9d`
- Characteristic with uuid: `68bf07fb-d00b-4c80-a796-f8be82b5dea`

The data is in a binary format:
- timestamp: `uint64_t`
- Values: `int16_t` for each value of each sensor (36) 
  *caution: signed integer*
  
Use with the ble-receiver script found in the `tools` folder.
