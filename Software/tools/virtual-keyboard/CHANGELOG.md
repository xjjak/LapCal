# Changes

- 2023-10-14 11:37: Check if virtual keyboard timestamps and the ESP timestamps are compatible. To do this I put some aluminum beside the virtual button and compare the position of the hardware detection and the merged virtual keyboard detection.

  *After adjusting the thresholds for the aluminum touching sensor the data shows that there the virtual keyboard seems the be about 125 millisecond faster than the hardware signal.*

  *My hypothesis is that there might be some delay when getting the current time on the ESP or something similar.*
- 2023-10-14 16:11: Plotting the actual data, the offset seems to match the activity of the sensor.
- 2023-10-20 23:01: Put python flask server in front of the virtual keyboard website. Further added functionality for client to share strokes live with the webserver. This may later be useful for traning the model live.
