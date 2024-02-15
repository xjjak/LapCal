#!/bin/bash

# sudo ./hid_listen | egrep --line-buffered "(0x[A-F0-9]+,)?[0-9]+,[0-9]+,[0-9]{1,2}" | tee -a keylog.csv
#
# pio device monitor | grep -E -o  --line-buffered "(^[0-9]+:)"

while true; do
    read line
    echo "$line$(date +%s%N)"
done
