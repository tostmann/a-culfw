import serial
import time

ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
time.sleep(2)
ser.write(b'V\r\n')
time.sleep(0.5)
print(f"V: {ser.read_all().decode().strip()}")

ser.write(b'X21\r\n')
time.sleep(0.5)
print(f"X21 response: {ser.read_all().decode().strip()}")

ser.write(b'C34\r\n')
time.sleep(1)
while ser.in_waiting:
    print(f"C34 response: {ser.readline().decode().strip()}")

ser.close()
