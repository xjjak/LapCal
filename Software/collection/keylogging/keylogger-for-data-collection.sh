#!/usr/bin/env nix-shell
#! nix-shell shell.nix -i python311

hid_listen=$(dirname "$0")/hid_listen
if [[ ! -e "$hid_listen" ]]; then
    hid_listen=hid_listen
fi

get_records(){
    $hid_listen | grep -E --line-buffered "(0x[A-F0-9]+,)?[0-9]+,[0-9]+,[0-9]{1,2}" 
}

prepend_timestamp(){
    while read line; do
        echo "$(($(date +%s%N) / 1000)),$line"
    done
}

get_records | prepend_timestamp
