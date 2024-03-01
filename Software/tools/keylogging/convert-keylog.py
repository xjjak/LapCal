DATA_OF_RIGHT_HAND = false # set to true to filter for right hand data, instead of left

# data entry format: [timestamp, keycode, row, col, layer(?), press_event, ...]
def preprocess(data):
    new_data = []
    for record in data:
        new_data.append([record[0], record[1], int(record[2]), int(record[3]), int(record[4]), bool(record[5])])
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

def get_data(file_name):
    data = []
    with open(file_name, "r") as file:
        for line in file:
            data.append(line.strip().split(','))
    return data

def get_key_number(row, col):
    number = None
    if not DATA_OF_RIGHT_HAND:
        if col <= 2:
            number = str(col)
        else:
            pass # Missing number conversion


    return int(number)

        
def generate_output(output_file_name):
    pass
    


data = get_data("keylog.csv")
