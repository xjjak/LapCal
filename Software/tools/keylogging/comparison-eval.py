import numpy as np

with open("comparing-timing.log", "r") as f:
    vals = []
    x, y  = map(int, f.readline().strip().split(":"))
    y = y // 1000
    print("Timestamp at start: ", y-x)
    for line in f:
        l, c = line.strip().split(":")
        l, c = int(l), int(c[:-3])
        vals.append(c-l)

a = sum(vals)/len(vals)
print("Averaged timestamp: ", a)
print("Difference between start and average: ", (y-x)-a)
print("Standard deviation: ", np.std(vals))
