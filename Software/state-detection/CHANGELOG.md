# Changes

- 2023-10-15 12:26: Setup dataset structure where each dataset has its own directory with
  1. `data.txt`: The readings coming from the glove.
  2. `clicks.txt`: The recorded clicks coming from the virtual keyboard application.
  3. `joined.txt`: Both files joined according to their timestamps. This file is to be generated by `merge.py`.