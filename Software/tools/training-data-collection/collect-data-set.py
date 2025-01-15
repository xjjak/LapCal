#!/bin/python3

import serial
import sys

key_pressed = False
counter = 0

headerline = "kb_state;ax;ay;az;gx;gy;gz\n"

if sys.argv[1] == "-f" or sys.argv[1] == "--file":
    output_file = sys.argv[2]
    with open(output_file, "w") as f:
        f.writelines(headerline)
        with serial.Serial('/dev/ttyUSB0', 115200, timeout=5) as ser:
            while True:
                line = ser.read_until(bytearray(b"\n\x04\x04"))
                # print(bytearray(line))
                data_vals = []
                data_vals.append(str(line[0]))
                for i in range(0,6):
                    s = 1+2*i
                    data_vals.append(str(int.from_bytes(line[s:s+2], byteorder='big', signed=True)))
                print(data_vals)
                print(';'.join(data_vals), flush=True, file=f)
            # if data_vals[0] != 0:
            #     if not key_pressed:
            #         key_pressed = True
            #     counter += 1
            # if key_pressed and data_vals[0] == 0:
            #     print("COUNTER: ", counter)
            #     counter = 0
            #     key_pressed = False

else:
    print("Specify output file name with -f/--file")
