import numpy as np
from pickle import dump


with open("data1.txt") as f:
    raw_data = f.read().strip()
    pressed = 0
    lines = raw_data.split("\n")
    data = np.zeros((8, len(lines)))
    
    for i, line in enumerate(lines):
        if "nt" in line:
            pressed = 0
        elif "t" in line:
            pressed = 1
        else:
            for j, d in enumerate(line.split(":")[0].split(";")):
                data[j][i] = float(d)
            data[-2][i] = pressed
            data[-1][i] = i

data_dict = dict()

data_dict["acc_x"] = data[0]
data_dict["acc_y"] = data[1]
data_dict["acc_z"] = data[2]
data_dict["gyr_x"] = data[3]
data_dict["gyr_y"] = data[4]
data_dict["gyr_z"] = data[5]
data_dict["class"] = data[6]
data_dict["time"]  = data[7]

with open("dataset/data1.pickle", "wb") as f:
    dump(data_dict, f)
