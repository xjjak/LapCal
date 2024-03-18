# Mainboard fimrware

## Build flags
- `RIGHT_HAND`: Set to `1` to reverse sensor order for right hand, `0` for the left hand. Throws error if not specified
- `MULT_CORE`: Set to `0` to disable parallel buffer resets, `1` to enable. Defaults to `1`.
- `BLE_SERIAL` Set to `0` to output sensor data to Serial Monitor, `1` enables ble-serial
