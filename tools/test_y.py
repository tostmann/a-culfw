import serial
import time

ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
time.sleep(2)
for _ in range(3):
    ser.write(b'y\r\n')
    time.sleep(0.5)
    while ser.in_waiting:
        print(ser.readline().decode().strip())
ser.close()
