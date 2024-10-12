#!/bin/python

import sys
import asyncio
from bleak import BleakClient

DATA_UUID = "68bf07fb-d00b-4c80-a796-f8be82b5dea7"

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
    print(out_str)


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
       

address = ""
micros_format = False

for j, i in enumerate(sys.argv[1:]):
    print(i)
    if i == "-d":
        address = sys.argv[j+2]
    elif i == "-m" or i == "--micros":
        micros_format = True
        
if address != "":
    asyncio.run(main(address))
else:
    print("Pass the MAC Address using flag '-d'")
