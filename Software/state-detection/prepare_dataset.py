import numpy as np
from bisect import bisect_right
from math import sin, cos

N_READINGS = 3 # number of readings considered by model
N_SENSORS = 6
N_VALUES = 3 + 3 + 3 # 3 acc + 3 gyr sin + 3 gyr cos
N_STATES = 17 # depends on keyboard
TARGET_NAMES = [(0,0), (1,0), (2,0), (3,0), (4,0),
                (0,1), (1,1), (2,1), (3,1), (4,1),
                (0,2), (1,2), (2,2), (3,2), (4,2),
                (0,3), (1,3)]

def readings_to_data(reading):
    sensors = reading.split(":")
    assert len(sensors) == N_SENSORS
    data = []
    
    for sensor in reading.split(":"):
        values = sensor.split(";")
        data.extend(
            [float(value) for value in values[:3]] +
            [sin(float(value)) for value in values[3:]] +
            [cos(float(value)) for value in values[3:]]
        )

    return data

def prepare_dataset(readings, clicks):
    n_samples  = len(readings) - N_READINGS
    n_features = N_READINGS * N_SENSORS * N_VALUES + N_STATES
    
    X = np.zeros(shape = (n_samples, n_features))
    y = np.zeros(shape = (n_samples, N_STATES))
    
    start = bisect_right(readings, clicks[0]) - N_READINGS
    for click_index in range(len(clicks)):
        flag, col, row = clicks[click_index][1].split(",")
        flag, col, row = bool(int(flag)), int(col), int(row)

        target = (col, row)
        index = TARGET_NAMES.index(target)
        y[start:, index] = 1 if flag else 0

        if click_index + 1 != len(clicks):
            start = bisect_right(readings, clicks[click_index+1]) - N_READINGS


    for sample_index in range(n_samples):
        for reading in range(N_READINGS):
            start = reading * (N_SENSORS * N_VALUES)
            end   = start + (N_SENSORS * N_VALUES)
            X[sample_index, start:end] = readings_to_data(readings[sample_index + reading][1])

        if sample_index == 0:
            X[sample_index, end:] = np.zeros(shape = (N_STATES,))
        else:
            X[sample_index, end:] = y[sample_index - 1]

    return (X, y)
