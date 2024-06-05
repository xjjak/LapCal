from prepare_dataset_abc import DataPreparation

import numpy as np
from bisect import bisect_right
from math import sin, cos

# Descriptive constants
N_SENSORS = 6
N_VALUES = 3 + 3 + 3  # 3 acc + 3 gyr sin + 3 gyr cos


class SingleTrigBuffer(DataPreparation):
    """=== TYPE B ===
    - 3 readings
    - sin/cos variants of rotation
    - labels are down presses"""

    def __init__(self, n_readings=3):
        self.n_readings = n_readings

    def get_name(self):
        return "single-trig-buffer"

    def reading_to_data(self, reading):
        """Convert a single reading to its semantic python equivalent."""

        sensors = reading.split(":")
        assert len(sensors) == N_SENSORS, f"{len(sensors)} Sensors???"
        data = []

        for sensor in reading.split(":"):
            # sensor malfunctioning
            if sensor == "-":
                data.extend([0] * 9)
            else:
                values = sensor.split(";")
                data.extend(
                    [float(value) for value in values[:3]]
                    + [sin(float(value)) for value in values[3:]]
                    + [cos(float(value)) for value in values[3:]]
                )

        return data

    def prepare_dataset(self, readings, clicks):
        """Process readings to features and labels (dataset)."""

        n_samples = len(readings) - self.n_readings
        n_features = self.n_readings * N_SENSORS * N_VALUES

        X = np.zeros(shape=(n_samples, n_features))
        y = np.zeros(shape=(n_samples,))  # FIXME: ",)" or ", 1)"

        # Labels
        clicks = list(filter(lambda x: x[0] < readings[n_samples - 1][0], clicks))
        for click_index in range(len(clicks)):
            flag, col, row = clicks[click_index][1].split(",")
            flag, col, row = bool(int(flag)), int(col), int(row)

            index = bisect_right(readings, clicks[click_index]) - self.n_readings
            if flag and index >= 0:
                y[index] = 1

        # Features
        for sample_index in range(n_samples):
            for reading in range(self.n_readings):
                start = reading * (N_SENSORS * N_VALUES)
                end = start + (N_SENSORS * N_VALUES)
                X[sample_index, start:end] = self.reading_to_data(
                    readings[sample_index + reading][1]
                )

        return (X, y)
