#!/bin/python

import sys
import os
import atexit
import asyncio
from bleak import BleakClient

DATA_UUID = "68bf07fb-d00b-4c80-a796-f8be82b5dea7"

def exit_handler():
    if output_to_file:
        sys.stdout.close()
        os.remove(file_path)


async def callback(sender, data: bytearray):
    #print(f"{data.hex()}")
    timestamp = int.from_bytes(data[:8], byteorder='big', signed=False)
    if not micros_format:
        out = [timestamp]
        out_str = str(timestamp) + ':'
    else:
        out = [timestamp * 1000] 
        out_str = str(timestamp) + "000:"
   
    for i in range(0,36):
        s = 8+2*i
        out.append(int.from_bytes(data[s:s+2], byteorder='big', signed=True))
        if (i+1) == 36:
            out_str += str(out[i+1])
        elif (i+1) % 6 == 0:
            out_str += str(out[i+1]) + ':'
        else:
            out_str += str(out[i+1]) + ';'
    print(out_str, flush=True)


async def main(address):
   client = BleakClient(address, services = ["f5aea478-9ec3-4bcf-af20-7f75e7c68c9d"]);
   try:
       await client.connect();
       await client.start_notify(DATA_UUID, callback)
       while True:
           await asyncio.sleep(0)
   except KeyboardInterrupt:
       exit()
   except Exception as e:
       print(e)
   finally:
       await client.disconnect()
       

if __name__ == '__main__':
    address = ""
    micros_format = False
    output_to_file = False
    file_path = ""

    for j, i in enumerate(sys.argv[1:]):
        if i == "-d":
            address = sys.argv[j+2]
        elif i == "-m" or i == "--micros":
            micros_format = True
        elif i == "-f" or i == "--file":
            output_to_file = True
            file_path = sys.argv[j+2]
            sys.stdout = open(file_path, 'wt')

    atexit.register(exit_handler)

    if address != "":
        asyncio.run(main(address))
    else:
        print("Pass the MAC Address using flag '-d'")
