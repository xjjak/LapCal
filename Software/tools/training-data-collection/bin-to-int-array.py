import serial

key_pressed = False
output = []
counter = 0
with serial.Serial('/dev/ttyUSB0', 115200, timeout=5) as ser:
    for i in range(0,10000):
        line = ser.readline()
        data_vals = []
        data_vals.append(line[0])
        for i in range(0,6):
            s = 1+2*i
            data_vals.append(int.from_bytes(line[s:s+2], byteorder='big', signed=True))
        print(data_vals)
        if data_vals[0] != 0:
            if not key_pressed:
                key_pressed = True
            counter += 1
        if key_pressed and data_vals[0] == 0:
            print("COUNTER: ", counter)
            counter = 0
            key_pressed = False

        output.append(data_vals)
