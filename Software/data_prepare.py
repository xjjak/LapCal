import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
from itertools import product
import random
import json

DATA_NAME = "imu_data"
LABEL_NAME = "event"

# takes df, and size of the form (before, after), as well as a
# specifier whether to consider press or release events
def prepare_original_data(data, file_to_read, window_size=(30, 20)):
    df = pd.read_csv(file_to_read)

    pressed      = df.kb_state != 0
    mask_press   = pressed & ~pressed.shift(1, fill_value=False)
    mask_release = pressed & ~pressed.shift(-1, fill_value=False)

    if "negative" in file_to_read.parent.name:
        event_index = filter(lambda x: x > window_size[0] and x < len(df) - window_size[1],
                             df[mask_press & mask_release].index)
        if len(list(event_index)) != 0:
            for idx in event_index:
                data_new = {}
                data_new["name"] = file_to_read.parent.name
                data_new[LABEL_NAME] = "negative"
                data_new[DATA_NAME] = df[["ax", "ay", "az", "gx", "gy", "gz"]].iloc[idx-window_size[0]:idx+window_size[1]].to_numpy().tolist()

                data.append(data_new)
        else:
            for idx in range(100, len(df)-window_size[1], 100):
                data_new = {}
                data_new["name"] = file_to_read.parent.name
                data_new[LABEL_NAME] = "negative"
                data_new[DATA_NAME] = df[["ax", "ay", "az", "gx", "gy", "gz"]].iloc[idx-window_size[0]:idx+window_size[1]].to_numpy().tolist()

                data.append(data_new)
    else:
        # press events
        press_index = filter(lambda x: x > window_size[0] and x < len(df) - window_size[1],
                             df[mask_press].index)

        for idx in press_index:
            data_new = {}
            data_new["name"] = file_to_read.parent.name
            data_new[LABEL_NAME] = "press"
            data_new[DATA_NAME] = df[["ax", "ay", "az", "gx", "gy", "gz"]].iloc[idx-window_size[0]:idx+window_size[1]].to_numpy().tolist()

            data.append(data_new)
            
        # release events
        release_index = filter(lambda x: x > window_size[0] and x < len(df) - window_size[1],
                               df[mask_release].index)

        for idx in release_index:
            data_new = {}
            data_new["name"] = file_to_read.parent.name
            data_new[LABEL_NAME] = "release"
            data_new[DATA_NAME] = df[["ax", "ay", "az", "gx", "gy", "gz"]].iloc[idx-window_size[0]:idx+window_size[1]].to_numpy().tolist()

            data.append(data_new)

def generate_negative_data(data, window_size = 50):
    """Generate negative data labeled as 'negative1~3'."""
    # Big movement -> around straight line
    for i in range(300):
        if i > 240:
            dic = {DATA_NAME: [], LABEL_NAME: "negative", "name": "negative3"}
        elif i > 180:
            dic = {DATA_NAME: [], LABEL_NAME: "negative", "name": "negative2"}
        else:
            dic = {DATA_NAME: [], LABEL_NAME: "negative", "name": "negative1"}
        start_ax = (random.random() - 0.5) * 2000
        start_ay = (random.random() - 0.5) * 2000
        start_az = (random.random() - 0.5) * 2000
        start_gx = random.random() * 20
        start_gy = random.random() * 20
        start_gz = random.random() * 20
        ax_increase = (random.random() - 0.5) * 10
        ay_increase = (random.random() - 0.5) * 10
        az_increase = (random.random() - 0.5) * 10
        gx_increase = random.random() - 0.5
        gy_increase = random.random() - 0.5
        gz_increase = random.random() - 0.5
        for j in range(window_size):
            dic[DATA_NAME].append([
                    start_ax + j * ax_increase + (random.random() - 0.5) * 6,
                    start_ay + j * ay_increase + (random.random() - 0.5) * 6,
                    start_az + j * az_increase + (random.random() - 0.5) * 6,
                    (start_gx + j * gx_increase + (random.random() - 0.5)) % 60 - 30,
                    (start_gy + j * gy_increase + (random.random() - 0.5)) % 60 - 30,
                    (start_gz + j * gz_increase + (random.random() - 0.5)) % 60 - 30,
            ])
        data.append(dic)
    # Random
    for i in range(300):
        if i > 240:
            dic = {DATA_NAME: [], LABEL_NAME: "negative", "name": "negative3"}
        elif i > 180:
            dic = {DATA_NAME: [], LABEL_NAME: "negative", "name": "negative2"}
        else:
            dic = {DATA_NAME: [], LABEL_NAME: "negative", "name": "negative1"}
        for j in range(window_size):
            dic[DATA_NAME].append([
                (random.random() - 0.5) * 1000,
                (random.random() - 0.5) * 1000,
                (random.random() - 0.5) * 1000,
                (random.random() - 0.5) * 20,
                (random.random() - 0.5) * 20,
                (random.random() - 0.5) * 20,
            ])
        data.append(dic)
    # Stay still
    for i in range(300):
        if i > 240:
            dic = {DATA_NAME: [], LABEL_NAME: "negative", "name": "negative3"}
        elif i > 180:
            dic = {DATA_NAME: [], LABEL_NAME: "negative", "name": "negative2"}
        else:
            dic = {DATA_NAME: [], LABEL_NAME: "negative", "name": "negative1"}
        start_x = (random.random() - 0.5) * 2000
        start_y = (random.random() - 0.5) * 2000
        start_z = (random.random() - 0.5) * 2000
        start_gx = (random.random() - 0.5) * 20
        start_gy = (random.random() - 0.5) * 20
        start_gz = (random.random() - 0.5) * 20
        for j in range(window_size):
            dic[DATA_NAME].append([
                start_ax + (random.random() - 0.5) * 40,
                start_ay + (random.random() - 0.5) * 40,
                start_az + (random.random() - 0.5) * 40,
                start_gx + (random.random() - 0.5),
                start_gy + (random.random() - 0.5),
                start_gz + (random.random() - 0.5),
            ])
        data.append(dic)

def write_data(data_to_write, path):
    with open(path, "w") as f:
        for item in data_to_write:
            dic = json.dumps(item, ensure_ascii=False)
            f.write(dic)
            f.write("\n")

datasets = [
    "2025-01-15_controlled-holds",
    "2025-01-15_controlled-taps",
    "2025-01-20_monkeytype",
    "2025-07-06_monkeytype",
    "2025-07-06_negative-monkeytype",
    "2025-07-06_negative-monkeytype2",
    "2025-07-07_monkeytype",
    "2025-07-07_monkeytype2",
]

if __name__ == '__main__':
    data = []
    data_dir = Path("_datasets/")
    for dataset in datasets:
        l = len(data)
        prepare_original_data(data, data_dir / 'raw' / dataset / 'centered.csv')
        print(f"{dataset}: {len(data)-l}")
    generate_negative_data(data)
    print("data_length: " + str(len(data)))
    write_data(data, data_dir / 'complete_data')
