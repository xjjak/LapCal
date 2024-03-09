#!/bin/bash

get_records(){
    ./hid_listen | grep -E --line-buffered "(0x[A-F0-9]+,)?[0-9]+,[0-9]+,[0-9]{1,2}" 
}

prepend_timestamp(){
    while read line; do
        echo "$(date +%s%N),$line"
    done
}

get_records | prepend_timestamp
