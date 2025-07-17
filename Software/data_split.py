from pathlib import Path
from data_prepare import write_data, DATA_NAME, LABEL_NAME
import json
import random
import os


data_dir = Path("_datasets/")

def read_data(path):
    data = []
    with open(path, 'r') as f:
        lines = f.readlines()
        for line in lines:
            dic = json.loads(line)
            data.append(dic)
    return data

def split_data(whole_data, train_names, valid_names, test_names):
    random.seed(30)
    random.shuffle(whole_data)
    train_data = []
    valid_data = []
    test_data = []
    for data in whole_data:
        if data["name"] in train_names:
            train_data.append(data)
        elif data["name"] in valid_names:
            valid_data.append(data)
        elif data["name"] in test_names:
            test_data.append(data)
    print(f'train_length: {len(train_data)} ({100*len(train_data)/len(whole_data):.1f}%)')
    print(f'valid_length: {len(valid_data)} ({100*len(valid_data)/len(whole_data):.1f}%)')
    print(f'test_length: {len(test_data)} ({100*len(test_data)/len(whole_data):.1f}%)')
    return train_data, valid_data, test_data
    

if __name__ == '__main__':
    data = read_data(data_dir / 'complete_data')
    data_press = list(filter(lambda d: d[LABEL_NAME] != 'release', data))
    train_names = [
        "2025-01-15_controlled-holds",
        "2025-01-15_controlled-taps",
        "2025-01-20_monkeytype",
        "2025-07-06_monkeytype",
        "2025-07-06_negative-monkeytype",
        "negative1",
    ]
    valid_names = [
        "2025-07-07_monkeytype2",
        "negative2",
    ]
    test_names = [
        "2025-07-07_monkeytype",
        "negative3",
        "2025-07-06_negative-monkeytype2",
    ]
    train_data, valid_data, test_data = split_data(data_press, train_names,
                                                   valid_names, test_names)
    os.makedirs(data_dir / 'split', exist_ok=True)
    write_data(train_data, data_dir / 'split' / 'train')
    write_data(valid_data, data_dir / 'split' / 'valid')
    write_data(test_data, data_dir / 'split' / 'test')
