#!/usr/bin/env nix-shell
#! nix-shell shell.nix -i python3

# TODO: (unit) tests

import sys
import os

from prepare_dataset_A import FullTrigBuffer
from prepare_dataset_B import SingleTrigBuffer

import joblib
from sklearn.model_selection import train_test_split

# Retrieve dataset type and load respective configuration
if "LAPCAL_DATASET_TYPE" in os.environ:
    datatype = os.environ["LAPCAL_DATASET_TYPE"]
else:
    datatype = input("Data type: ")
    

# Setup dataset preparation object according to dataset type
match datatype:
    case "A":
        prepare_dataset = FullTrigBuffer()
    case "B":
        prepare_dataset = SingleTrigBuffer()
    case _:
        print(f"ERROR: No configuration for dataset type [{datatype}] found.")
        exit(1)


# Check if dataset directory was provided
if len(sys.argv) > 1:
    dataset_directory = sys.argv[1]
    assert os.path.exists(
        os.path.join(dataset_directory, "data.txt")
    ) and os.path.exists(
        os.path.join(dataset_directory, "clicks.txt")
    ), f"Could not find data.txt or clicks.txt in {dataset_directory}"
else:
    print("ERROR: No directory specified.")
    exit(1)


# Read readings from data.txt
readings_with_time = []
with open(os.path.join(dataset_directory, "data.txt")) as f:
    data = f.read().split("\n")

    # Iterate through all entries except
    # - the first, because it only contains the column names or partial information,
    # - the last, because it may be empty or contain partial information.
    for line in data[1:-1]:
        reading = line.split(":")
        readings_with_time += [(int(reading[0]), ":".join(reading[1:]))]

readings_with_time.sort()

# Read clicks from clicks.txt
clicks_with_time = []
with open(os.path.join(dataset_directory, "clicks.txt")) as f:
    for line in f.read().strip().split("\n"):
        click = line.split(",")
        clicks_with_time += [(int(click[0]), ",".join(click[1:]))]

clicks_with_time.sort()

# Write result to joined.txt
joined = sorted(readings_with_time + clicks_with_time)
with open(os.path.join(dataset_directory, "joined.txt"), "w") as f:
    for i in joined:
        f.write(i[1] + "\n")


# Save a dataset for every preparation function in the given module
x, y = prepare_dataset(readings_with_time, clicks_with_time)

# Split into train, dev and test distribution
x_train, x_dev, y_train, y_dev = train_test_split(
        x, y, test_size=0.3, random_state=25
)
x_dev, x_test, y_dev, y_test = train_test_split(
        x_dev, y_dev, test_size=0.4, random_state=16
)


# Write dataset
dataset = (x_train, y_train, x_dev, y_dev, x_test, y_test)
joblib.dump(dataset, os.path.join(dataset_directory, f"dataset-{prepare_dataset.get_name()}.joblib"))
