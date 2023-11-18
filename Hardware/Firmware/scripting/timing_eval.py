from statistics import mean

file_name = "../mainboard/timing_data.dat"

with open(file_name, "r") as file:
    stripped_lines = [line.strip() for line in file]

def val_from_line(l):
    return int(line.split(": ")[1])

mux_vals = []
buf_reset_vals = []
buf_wait_vals = []
buf_read_vals = []
whole_reading_vals = []
writing_vals = []


for line in stripped_lines:
    if line.startswith("After mux"):
        mux_vals.append(val_from_line(line))
    elif line.startswith("Before get"):
        buf_reset_vals.append(val_from_line(line))
    elif line.startswith("before bytes"):
        buf_wait_vals.append(val_from_line(line))
    elif line.startswith("after bytes"):
        buf_read_vals.append(val_from_line(line))
    elif line.startswith("All readings"):
        whole_reading_vals.append(val_from_line(line))
    elif line.startswith("after writing"):
        writing_vals.append(val_from_line(line))
    

print(f"Avg. Mux switch (%d): %d" % (len(mux_vals), mean(mux_vals)))
print(f"Avg. Buf reset (%d): %d" % (len(buf_reset_vals), mean(buf_reset_vals)))
print(f"Avg. Buf wait (%d): %d" % (len(buf_wait_vals), mean(buf_wait_vals)))
print(f"Avg. Buf read (%d): %d" % (len(buf_read_vals), mean(buf_read_vals)))
print(f"Avg. reading of all sensors (%d): %d" % (len(whole_reading_vals), mean(whole_reading_vals)))
print(f"Avg. After reading (%d): %d" % (len(writing_vals), mean(writing_vals)))
