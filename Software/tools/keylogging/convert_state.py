translation = ["00","10","20", "01","11","21", "02","12","22",
               "03","13","23", "04","14","24", "30","31"]

# initiliaze state
state = [0,]*17

while True:
    # read and split input 
    line = input()
    columns = line.split(",")

    # map input to key
    key = columns[2] + columns[3]
    # skip input if invalid key
    if not key in translation:
        continue
    index = translation.index(key)

    # change key state according to input
    if columns[5] == "1":
        state[index] = 1
    else:
        state[index] = 0

    # print state
    print(state)
    
