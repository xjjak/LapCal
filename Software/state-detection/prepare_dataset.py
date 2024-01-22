import numpy as np
from bisect import bisect_right
from math import sin, cos

N_READINGS = 3 # number of readings considered by model
N_SENSORS = 6
N_VALUES = 3 + 3 + 3 # 3 acc + 3 gyr sin + 3 gyr cos
N_STATES = 17 # depends on keyboard
TARGET_NAMES = ["00","01","02",                 "10","11","12",  "20","21","22",
                "30","31","32","33","34","35",  "40","41","42",  "50","51"]

def readings_to_data(reading):
    sensors = reading.split(":")
    assert len(sensors) == N_SENSORS
    data = []
    
    for sensor in reading.split(":"):
        # sensor malfunctioning
        if sensor == "-":
            data.extend([0]*9)
        else:
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
        flag, target = clicks[click_index][1].split()

        index = TARGET_NAMES.index(target)
        y[start:, index] = 1 if flag == "+" else 0

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
