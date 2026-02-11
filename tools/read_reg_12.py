import serial
import time
s = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
s.write(b'C12\n')
time.sleep(0.1)
print(f"MDMCFG2: {s.read_all().decode().strip()}")
s.close()
