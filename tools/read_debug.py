import serial
import time

s = serial.Serial('/dev/ttyACM0', 115200, timeout=2)
s.write(b'y\n')
print(s.read_until(b'\n').decode())
print(s.read_until(b'\n').decode())
s.close()
