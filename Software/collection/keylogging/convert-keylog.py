#!/usr/bin/env nix-shell
#! nix-shell shell.nix -i python311

# set to true to filter for right hand data, instead of left
DATA_OF_RIGHT_HAND = False

# List for translation from 
LEFT_HAND  = {(0,0), (1,0), (2,0), (3,0), (4,0),
              (0,1), (1,1), (2,1), (3,1), (4,1),
              (0,2), (1,2), (2,2), (3,2), (4,2),
              (0,3), (1,3)}
RIGHT_HAND = {(0,4), (1,4), (2,4), (3,4), (4,4),
              (0,5), (1,5), (2,5), (3,5), (4,5),
              (0,6), (1,6), (2,6), (3,6), (4,6),
              (0,7), (1,7)}


# data entry format: [timestamp, keycode, row, col, layer(?), press_event, ...]
def preprocess(record):
    return [
        int(record[0]), # [0] timestamp
        record[1],      # [1] keycode
        int(record[2]), # [2] row
        int(record[3]), # [3] col
        int(record[4]), # [4] layer
        bool(int(record[5])) # [5] press_event
    ]


def filter_for_hand(record):
    global DATA_OF_RIGHT_HAND
    global LEFT_HAND
    global RIGHT_HAND
    
    pos = (record[3], record[2])
    
    if not DATA_OF_RIGHT_HAND:
        if pos in LEFT_HAND:
            return True
    else:
        if pos in RIGHT_HAND:
            return True

    return False
        

# FIXME: unused
def get_key_number(row, col):
    global DATA_OF_RIGHT_HAND
    global LEFT_HAND
    global RIGHT_HAND

    pos = (row, col)
    
    if not DATA_OF_RIGHT_HAND:
        number = LEFT_HAND.index(pos)
    else:
        number = RIGHT_HAND.index(pos)
        
    return number


if __name__ == "__main__":
    while True:
        try:
            line = input()
        except EOFError:
            break

        record = line.split(",")
        record = preprocess(record)

        if not filter_for_hand(record):
            continue
        
        time, flag, col, row = record[0], record[5], record[3], record[2]

        print(f"{time},{int(flag)},{col},{row}")
