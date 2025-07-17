import sys

if len(sys.argv) == 2:
    data_name = sys.argv[1]
else:
    data_name = input("Dataset Name: ")

#!/usr/bin/env python3

from pathlib import Path
import os

# NOTE: This program does only work on new datasets within the following dataset directory!

data_dir = Path("_datasets/raw/")
assert os.path.isdir(data_dir), "Error: Cannot find dataset directory."
assert data_name, "Error: No dataset given."
data_path = data_dir / data_name
assert os.path.isdir(data_path), f"Error: Cannot find given dataset \"{data_path}\"" 

# Import necessary libraries
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

print("Finding incosistencies within the dataset...")
# Load the dataset using pandas
df = pd.read_csv(data_path/'raw.csv', sep=';', dtype=np.float64)
df.dtypes
df = df.astype({'kb_state': np.int64})
df.dtypes
df.drop(index=df.index[0], inplace=True)
df.reset_index(drop=True, inplace=True)
df.to_csv(data_path/'formatted.csv', index=False)
# Get starting (inclusive) and ending (exclusive) index of reading
# around given reading.
def get_tap_around(reading_idx, df):
    # check if inside tap
    assert df.kb_state[reading_idx] != 0
    
    i = reading_idx
    while i > 0 and df.kb_state[i-1] != 0:
        i -= 1
    a = i
    while i < len(df) and df.kb_state[i] != 0:
        i += 1
    b = i
    
    return a, b
idx = 0
taps = list()
while idx < len(df):
    if df.kb_state[idx] != 0:
        tap = get_tap_around(idx, df)
        taps.append(tap)
        idx = tap[1]
    else:
        idx += 1
valid_keyboard_states = [0, 1, 2, 4]
def is_tap_valid(tap, df):
    return len(set(map(lambda i: df.kb_state[i], range(*tap)))) == 1 \
        and df.kb_state[tap[0]] in valid_keyboard_states
taps_err = list(filter(lambda tap: not is_tap_valid(tap, df), taps))

# Print overview of all erroneous taps
print("Erroneous taps:")
for tap in sorted(taps_err):
    start, end = tap
    states = set()
    for j in range(start, end):
        states.add(df.kb_state[j])
    print(f"  from {tap[0]:6d} to {tap[1]:6d} with states: {', '.join(map(str, states))}")
print("Fixing incosistencies...")
reassignments = dict()

for tap in taps_err:
    start, end = tap
    contains_valid = False
    for i in range(start, end):
        if df.kb_state[i] in valid_keyboard_states:
            assignment = df.kb_state[i]
            contains_valid = True
            break
    if not contains_valid:
        if df.kb_state[start] & 1:
            assignment = 1
        elif df.kb_state[start] & 4:
            assignment = 4
        else:
            assignment = 2

    reassignments[tap] = assignment
for tap, assignment in reassignments.items():
    start, end = tap
    for i in range(start, end):
        df.loc[0,"kb_state"] = assignment
df.to_csv(data_path/'fixed.csv', index=False)
print("Centering the data using an EMA...")
def center_moving_average(df, alpha=0.995):
    df = df.copy()
    df.iloc[:,1:] = df.iloc[:,1:] - df.iloc[:,1:].ewm(alpha=1-alpha).mean()
    return df

df_center = center_moving_average(df, alpha=0.995)
df_center.to_csv(data_path/'centered.csv', index=False)

print("Preprocessing successful!")
