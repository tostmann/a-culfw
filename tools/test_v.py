import serial
import time
import sys

port = '/dev/ttyACM0'
baud = 115200

try:
    ser = serial.Serial(port, baud, timeout=2)
    time.sleep(1)
    ser.write(b'V\n')
    time.sleep(0.5)
    response = ser.read_all().decode()
    print(f"Response: {response}")
    ser.close()
except Exception as e:
    print(f"Error: {e}")
