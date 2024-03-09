# set to true to filter for right hand data, instead of left
DATA_OF_RIGHT_HAND = False

# List for translation from 
TRANSLATION = dict(zip(
    ["00","10","20", "01","11","21", "02","12","22",
     "03","13","23", "04","14","24", "31","30"],
    ["00","01","02", "10","11","12", "20","21","22",
     "30","31","32", "40","41","42", "50","51"]
))


# data entry format: [timestamp, keycode, row, col, layer(?), press_event, ...]
def preprocess(data):
    new_data = []
    for record in data:
        new_data.append([
            record[0],
            record[1],
            int(record[2]),
            int(record[3]),
            int(record[4]),
            bool(record[5])
        ])
    return new_data


def filter_for_hand(data):
    global DATA_OF_RIGHT_HAND
    filtered_data = []
    for record in data:
        if DATA_OF_RIGHT_HAND:
            if record[3] > 4:
                filtered_data.append(record)
        else:
            if record[3] < 5:
                filtered_data.append(record)

    return filtered_data

# def get_data(file_name=None):
#     data = []
#     if file_name:
#         with open(file_name, "r") as f:
#             for line in f.read().strip().split('\n'):
#                 data.append(line.strip().split(','))
#     else:
        
        
#     return data

def get_key_number(row, col):
    global TRANSLATION

    if not DATA_OF_RIGHT_HAND:
        key = str(row) + str(col)
        number = TRANSLATION[key]
    else:
        # TODO: right hand inversion
        
    return number


if __name__ == "__main__":
    while True:
        try:
            line = input()
        except EOFError:
            break

        record = preprocess(line)
        record = filter_for_hand(record)

        number = get_key_number(record[2], record[3])
        if record[5]:
            print(f"+ {number}")
        else:
            print(f"- {number}")
