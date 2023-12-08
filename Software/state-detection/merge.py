import sys
import os
import numpy as np
import joblib
from prepare_dataset import prepare_dataset

# check if dataset directory was provided
if len(sys.argv) > 1:
    dataset_directory = sys.argv[1]
    assert os.path.exists(os.path.join(dataset_directory, "data.txt")) \
        and os.path.exists(os.path.join(dataset_directory, "clicks.txt"))
else:
    print("No directory specified.")
    exit()


milliseconds = 0
readings_with_time = []

# read readings from data.txt
with open(os.path.join(dataset_directory, "data.txt")) as f:
    data = f.read().split("\n")
    milliseconds = int(data[0].split(":")[0])
    last_milliseconds = milliseconds

    # Iterate through all entries except
    # - the first, because it only contains the column names,
    # - the last, because it may be empty or contain partial information.
    for line in data[1:-1]:
        if "t" in line:
            readings_with_time += [(last_milliseconds+1, line)]
        else:
            reading = line.split(":")[:-1]
            last_milliseconds = milliseconds + int(reading[0])//1000
            readings_with_time += [(last_milliseconds, ":".join(reading[1:]))]

clicks_with_time  = []

# read clicks from clicks.txt
with open(os.path.join(dataset_directory, "clicks.txt")) as f:
    for line in f.read().strip().split("\n"):
        click = line.split()
        clicks_with_time += [(int(click[0])+125, " ".join(click[1:]))]


# join readings and clicks
joined = sorted(readings_with_time +  clicks_with_time)

# write result to joined.txt
with open(os.path.join(dataset_directory, "joined.txt"), "w") as f:
    for i in joined:
        f.write(i[1]+"\n")


dataset = prepare_dataset(readings_with_time, clicks_with_time)
joblib.dump(dataset, os.path.join(dataset_directory, "dataset.joblib"))
