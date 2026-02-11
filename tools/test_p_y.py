import serial
import time

s = serial.Serial('/dev/ttyACM0', 115200, timeout=2)
s.write(b'P\n')
time.sleep(1)
s.write(b'y\n')
print(s.read_all().decode())
s.close()
