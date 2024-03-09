# Usage

1. Provide `hid_listen` script in the local folder.
2. Either:
   - If you want to keep the `keylog.csv` file separately.
     1. Run `./keylogger-for-data-collection.sh >> keylog.csv` while collecting data.
     2. Run `python convert-keylog.py < keylog.csv > clicks.txt` to get the format required by the `prepare_dataset.py` utility.
   - If you don't care about the `keylog.csv` file you can skip it and just run the command `./keylogger-for-data-collection.sh | python convert-keylog.py > clicks.txt`.
