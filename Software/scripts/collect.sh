#!/usr/bin/env nix-shell
#! nix-shell shell.nix -i bash

secho () {
    echo -e "\033[0;36m$(basename "$0")\033[0m: $*"
}

# (Environment) variables with default
export LAPCAL_DATASET_TYPE=${LAPCAL_DATASET_TYPE:-A}
export LAPCAL_BLE=${LAPCAL_BLE:-1}
export LAPCAL_BLE_DEVICE=${LAPCAL_BLE_DEVICE:-lapcal}

error="\033[0;35mERROR\033[0m"

# Argument parsing
while test $# != 0
do
    case "$1" in
        -B|--no-bluetooth)
            LAPCAL_BLE=0 ;;
        -n|--name)
            LAPCAL_BLE_DEVICE=$2; shift ;;
        -d|--data)
            LAPCAL_DATA_DIR=$2; shift ;;
        # -r) # readings
        -t|--type)
            LAPCAL_DATASET_TYPE=$2; shift ;;
       

        
        # -n) no_update_info=t ;;
        # -a) all_into_one=t ;;
        # -A) all_into_one=t
        #     unpack_unreachable=--unpack-unreachable ;;
        # -d) remove_redundant=t ;;
        # -q) GIT_QUIET=t ;;
        # -f) no_reuse=--no-reuse-object ;;
        # -l) local=--local ;;
        # --max-pack-size|--window|--window-memory|--depth)
        #     extra="$extra $1=$2"; shift ;;
        # --) shift; break;;
        # *)  usage ;;
    esac
    shift
done


# Check if data dir exiss
if [[ -z "$LAPCAL_DATA_DIR" ]]; then
    secho "$error: No data storage location specified. You have to either set LAPCAL_DATA_DIR or pass it with --data."
    exit 1
elif [[ ! -d "$LAPCAL_DATA_DIR" ]]; then
    secho "$error: Directory \"$LAPCAL_DATA_DIR\" does not exist."
    exit 1
fi

# Set and create dataset directory
dataset_dir="$LAPCAL_DATA_DIR/dataset/$LAPCAL_DATASET_TYPE$(date +"%Y%m%d%H%M%S")"
secho "Creating dataset directory: $dataset_dir"
mkdir -p $dataset_dir

glove_file=$dataset_dir/data.txt
keylog_csv_file=$dataset_dir/keylog.csv
keylog_file=$dataset_dir/clicks.txt

keylog_translation_script=$(dirname "$0")/../collection/keylogging/keylogger-for-data-collection.sh
keylog_conversion_script=$(dirname "$0")/../collection/keylogging/convert-keylog.py
merge_script=$(dirname "$0")/../processing/merge.py

error_flag=0

# Processing procedure
process () {
    if [[ "error_flag" -eq "0" ]]; then
        secho "Processing generated data..."
        secho "  running merge.py..."
        if $merge_script $dataset_dir; then
            secho "  ...success"
        else
            secho "$error: Merge failed."
        fi
    fi
}


if [[ $LAPCAL_BLE -eq 1 ]]; then
    
    secho "Using bluetooth serial."
    
    # Setup bluetooth serial connection
    secho "Scanning for bluetooth devices matching \"$LAPCAL_BLE_DEVICE\""
    device=$(ble-scan | rg -i $LAPCAL_BLE_DEVICE | head -n 1) #  | cut -f 1 -d " "
    if [[ -z "$device" ]]; then
        secho "$error: No bluetooth device containing \"$LAPCAL_BLE_DEVICE\" found."
        exit 2
    fi

    secho "Connecting to device \"$(echo $device | cut -f 3- -d " ")\"..."
    (sleep 1; ble-serial -p /tmp/ttyBLE -d "$(echo $device | cut -f 1 -d ' ')") & ble_pid=$!

    # Setup shutdown trap with proper clean up
    trap "process; secho \"Shutting down...\"; kill -SIGTERM $ble_pid && (rm /tmp/ttyBLE; bluetoothctl disconnect \"$(echo $device | cut -f 1 -d ' ')\")" EXIT

    sleep 6

    if [[ -e "/tmp/ttyBLE" ]]; then
        secho "Connected sucessfully! (PID $ble_pid)"
    else
        secho "$error: Could not connect to the device."
        exit 2
    fi


    secho "Piping serial input into \"$glove_file\""
    pio device monitor -p /tmp/ttyBLE | rg -v "^---" > "$glove_file" &
    
else
    
    secho "Using regular serial input."
    secho "$error: This feature is not yet implemented."
    exit 3

    trap "process" EXIT
    
    # TODO: some more env variables...
    
    pio device monitor -p /tmp/ttyBLE | rg -v "^---" > "$glove_file" &
    
fi


# Start keylogger and processing script
secho "Starting keylogger... "
sh "$keylog_translation_script" | tee "$keylog_csv_file" | python "$keylog_conversion_script" > "$keylog_file" &
secho "Piping output into \"$keylog_file\" and the intermediary output into \"$keylog_csv_file\""

secho "\033[0;32mReady!\033[0m Collecting data..."
while true; do
    sleep 5

    # TODO: test
    # count available sensors
    line=$(tail -n 2 "$glove_file" | head -n 1)
    IFS=':' read -ra parts <<< "$line"
    count=0
    for part in "${parts[@]}"; do
        if [[ "$part" != "-" ]]; then
            ((count++))
        fi
    done
    # notify of missing sensors
    if (( $count != 7 )); then
        notify-send "Missing sensors" "Only $count seem to work properly." -u critical
    fi

    # Check for bluetooth error and push notification on error
    if (( "$(date +%s)" >= "$(date -r "$glove_file" +%s)+10" )); then
        secho "$error: Bluetooth serial stopped working."
        notify-send "Bluetooth Error" "Bluetooth serial stopped working." -u critical
        error_flag=1
        kill -SIGINT 0
    fi
done
