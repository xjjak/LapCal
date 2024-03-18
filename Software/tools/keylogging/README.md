# Usage

1. Provide `hid_listen` script in the local folder.
2. Either:
   - If you want to keep the `keylog.csv` file separately.
     1. Run `./keylogger-for-data-collection.sh >> keylog.csv` while collecting data.
     2. Run `python convert-keylog.py < keylog.csv > clicks.txt` to get the format required by the `prepare_dataset.py` utility.
   - If you don't care about the `keylog.csv` file you can skip it and just run the command `./keylogger-for-data-collection.sh | python convert-keylog.py > clicks.txt`.

## QMK Firmware modificiations
*Based upon [this tutorial](https://precondition.github.io/qmk-heatmap#keyboard-firmware-setup)*

- Follow [basic qmk instructions](https://docs.qmk.fm/#/newbs) for building and flashing
- locate the folder with keymap.json/keymap.c
- *If there is only keymap.json*:
   - run `qmk json2c keymap.json > keymap.c`
   - remove/rename the .json
- change the bootloader in `rules.mk` to `atmel-dfu`
- append `CONSOLE_ENABLE= = yes` to `rules.mk`
- Append the following to `keymap.c`:
  ```c
  bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
      #ifdef CONSOLE_ENABLE
          const bool is_combo = IS_COMBOEVENT(record->event); // DIFFERS FROM TUTORIAL
          uprintf("0x%04X,%u,%u,%u,%b,0x%02X,0x%02X,%u\n",
               keycode,
               is_combo ? 254 : record->event.key.row,
               is_combo ? 254 : record->event.key.col,
               get_highest_layer(layer_state),
               record->event.pressed,
               get_mods(),
               get_oneshot_mods(),
               record->tap.count
               );
      #endif
      // switch (keycode) {
      // //...
      // }
      return true;
  }
  ```
- Now try compilation with `qmk compile` (assuming standard kb and km are set)

- If there is a compilation error like this:
```
Compiling: platforms/avr/drivers/i2c_master.c                                                      In function 'i2c_start_impl',
    inlined from 'i2c_start' at platforms/avr/drivers/i2c_master.c:104:18:
platforms/avr/drivers/i2c_master.c:61:10: error: array subscript 0 is outside array bounds of 'volatile uint8_t[0]' {aka 'volatile unsigned char[]'} [-Werror=array-bounds=]
   61 |     TWCR = 0;
      |          ^
```
Try appending `-e AVR_CFLAGS="-Wno-array-bounds"` to `qmk compile` or `qmk flash` as a workaround.

# Documenation


## Timing comparison
Putting the micros beside the computer timestamp and using a python script to evaluate:

```
Timestamp at start (host-micros):  1708005405495278
Averaged timestamp:  1708005405496783.5
Difference between first timestamp and average timestamp:  -1505.5
Standard deviation:  887.4216428542204
```
