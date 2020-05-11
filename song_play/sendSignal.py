import numpy as np
import serial
import time


waitTime = 0.1
signalLength = 32
# Generate signal table
signalTable = [261, 293, 329, 349, 391, 440, 493, 523,
261, 293, 329, 349, 391, 440, 493, 523,
261, 293, 329, 349, 391, 440, 493, 523,
261, 293, 329, 349, 391, 440, 493, 523
]
# output formatter
formatter = lambda x: "%d" % x

# send the waveform table to K66F
serdev = '/dev/ttyACM0'
s = serial.Serial(serdev)
print("Sending signal ...")
print("It may take about %d seconds ..." % (int(signalLength * waitTime)))
for data in signalTable:
    s.write(bytes(formatter(data), 'UTF-8'))
    time.sleep(waitTime)

s.close()

print("Signal sended")