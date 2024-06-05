from prepare_dataset_abc import DataPreparation

import numpy as np
from bisect import bisect_right
from math import sin, cos

DEFAULT_TARGET_NAMES = [
    (0, 0),
    (1, 0),
    (2, 0),
    (3, 0),
    (4, 0),
    (0, 1),
    (1, 1),
    (2, 1),
    (3, 1),
    (4, 1),
    (0, 2),
    (1, 2),
    (2, 2),
    (3, 2),
    (4, 2),
    (0, 3),
    (1, 3),
]

# Descriptive constants
N_SENSORS = 6
N_VALUES = 3 + 3 + 3  # 3 acc + 3 gyr sin + 3 gyr cos


class FullTrigBuffer(DataPreparation):
    """=== TYPE A ===
    - 3 readings
    - sin/cos variants of rotation
    - labels are full state (17 keys)"""

    def __init__(self, n_readings=3, n_states=17, target_names=DEFAULT_TARGET_NAMES):
        self.n_readings = n_readings
        self.n_states = n_states
        self.target_names = target_names

    def get_name(self):
        return "full-trig-buffer"

    def reading_to_data(self, reading):
        """Convert a single reading to its semantic python equivalent."""
        sensors = reading.split(":")
        assert len(sensors) == N_SENSORS
        data = []

        for sensor in reading.split(":"):
            # Sensor malfunctioning
            if sensor == "-":
                data.extend([0] * 9)
            else:
                values = sensor.split(";")
                # Per sensor add to data:
                # - acceleration
                # - vertical compononent (sin) of rotation
                # - horizontal component (cos) of rotation
                data.extend(
                    [float(value) for value in values[:3]]
                    + [sin(float(value)) for value in values[3:]]
                    + [cos(float(value)) for value in values[3:]]
                )

        return data

    def prepare_dataset(self, readings, clicks):
        """Process readings to features and labels (dataset)."""

        n_samples = len(readings) - self.n_readings
        n_features = self.n_readings * N_SENSORS * N_VALUES + self.n_states

        X = np.zeros(shape=(n_samples, n_features))
        y = np.zeros(shape=(n_samples, self.n_states))

        # Prepare labels
        start = bisect_right(readings, clicks[0]) - self.n_readings
        for click_index in range(len(clicks)):
            flag, col, row = clicks[click_index][1].split(",")
            flag, col, row = bool(int(flag)), int(col), int(row)

            target = (col, row)
            index = self.target_names.index(target)
            y[start:, index] = 1 if flag else 0

            if click_index + 1 != len(clicks):
                start = (
                    bisect_right(readings, clicks[click_index + 1]) - self.n_readings
                )

        # Prepare features
        for sample_index in range(n_samples):
            for reading in range(self.n_readings):
                start = reading * (N_SENSORS * N_VALUES)
                end = start + (N_SENSORS * N_VALUES)
                X[sample_index, start:end] = self.reading_to_data(
                    readings[sample_index + reading][1]
                )

            if sample_index == 0:
                X[sample_index, end:] = np.zeros(shape=(self.n_states,))
            else:
                X[sample_index, end:] = y[sample_index - 1]

        return (X, y)
