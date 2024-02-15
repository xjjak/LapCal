import numpy as np

with open("comparing-timing.log", "r") as f:
    vals = []
    for line in f:
        l, c = line.strip().split(":")
        l, c = int(l), int(c[-11:-3])
        vals.append(c-l)

print(sum(vals)/len(vals))
print(np.std(vals))
