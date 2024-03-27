import numpy as np
from bisect import bisect_right
from math import sin, cos

# Constants subject to change
N_READINGS = 3 # number of readings considered by model

# Descriptive constants
N_SENSORS = 6
N_VALUES = 3 + 3 + 3 # 3 acc + 3 gyr sin + 3 gyr cos


def reading_to_data(reading):
    sensors = reading.split(":")
    assert len(sensors) == N_SENSORS, f"{len(sensors)} Sensors???"
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


def prepare_dataset_press(readings, clicks):
    n_samples  = len(readings) - N_READINGS
    n_features = N_READINGS * N_SENSORS * N_VALUES
    
    X = np.zeros(shape = (n_samples, n_features))
    y = np.zeros(shape = (n_samples,)) # FIXME: ",)" or ", 1)"

    # Labels
    clicks = list(filter(
        lambda x: x[0] < readings[n_samples-1][0],
        clicks
    ))
    for click_index in range(len(clicks)):
        flag, col, row = clicks[click_index][1].split(",")
        flag, col, row = bool(int(flag)), int(col), int(row)

        index = bisect_right(readings, clicks[click_index]) - N_READINGS
        if flag and index >= 0:
            y[index] = 1

    # Features
    for sample_index in range(n_samples):
        for reading in range(N_READINGS):
            start = reading * (N_SENSORS * N_VALUES)
            end   = start + (N_SENSORS * N_VALUES)
            X[sample_index, start:end] = reading_to_data(readings[sample_index + reading][1])

    return (X, y)


# --- EXPORT ---
export = [
    prepare_dataset_press
]
