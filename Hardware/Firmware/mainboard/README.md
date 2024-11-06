
# Mainboard firmware

## Build flags
- `RIGHT_HAND`: Set to `1` to reverse sensor order for right hand, `0` for the left hand. Throws error if not specified
- `MULT_CORE`: Set to `0` to disable parallel buffer resets, `1` to enable. Defaults to `1`.
- `BLE`: Set to `0` to output sensor data to Serial Monitor, `1` enables ble server and data transmission (Requires MULT_CORE=0)

## Setting build flags
### PlatformIO environments
The easiest method (and safest) is using the available pio environments:
- `left` / `right` : Data is printed to serial monitor (MULT_CORE=0)
- `left-ble` / `right-ble` : (BLE=1 MULT_CORE=0)
by using `pio run -e [left/right/left-ble/right-ble]`.
Defaults to `left` when `-e` is omitted.

*(left/right refers to the targeted hand)*

###  Environment variable override
To customize build flags when needed, prepend something like:
`PLATFORMIO_BUILD_FLAGS="-DMULT_CORE=[0/1] -DBLE=[0/1] -DRIGHT_HAND=[0/1]`
to a pio run command.
Any Flag that can stay the same as in the environment can be omitted.
This should override potentially differing flags set in the environment.

## BLE Server 
When BLE is enabled the ESP32 acts as an BLE Server providing
- Service with uuid: `f5aea478-9ec3-4bcf-af20-7f75e7c68c9d`
- Characteristic with uuid: `68bf07fb-d00b-4c80-a796-f8be82b5dea`

The data is in a binary format:
- timestamp: `uint64_t`
- Values: `int16_t` for each value of each sensor (36) 
  *caution: signed integer*
  
Use with the ble-receiver script found in the `Software/tools` folder.

