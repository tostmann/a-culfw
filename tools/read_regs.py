import serial
import time

s = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
s.write(b'C35\n')
time.sleep(0.1)
print(f"MARCSTATE: {s.read_all().decode().strip()}")

s.write(b'C00\n')
time.sleep(0.1)
print(f"IOCFG2: {s.read_all().decode().strip()}")

s.write(b'C02\n')
time.sleep(0.1)
print(f"IOCFG0: {s.read_all().decode().strip()}")

s.close()
