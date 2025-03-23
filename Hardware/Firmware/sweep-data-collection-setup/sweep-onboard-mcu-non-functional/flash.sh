#!/bin/zsh
pio run
dfu-programmer atmega32u4 erase --force
dfu-programmer atmega32u4 flash --force .pio/build/32u416m/firmware.hex
