# BLE-RECEIVER
This script can be used as a drop-in replacement for ble-serial for receiving data from the mainboard in the new binary format and translating it into the established format.

*Requires the [bleak][https://github.com/hbldh/bleak] python module.*

## Flags
- `-d`: Specify the MAC-Address for the BLE-Server
- Optional: 
  - `-m, --micros`: Add 3 zeros to the end of the timestamp for microsecond format. This is inaccurate though.
  - `-f, --file` All output is redirected to file at give path. (Useful for compatiblity with live-plotting by passing `/tmp/ttyBLE`)
