import serial
import time
import sys

s_ref = serial.Serial('/dev/ttyACM1', 38400, timeout=1)
s_esp = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

s_ref.write(b'X67\n')
time.sleep(0.5)
s_ref.read_all()

print("Triggering P on ESP32...")
s_esp.write(b'P\n')

start = time.time()
while time.time() - start < 2:
    line = s_ref.readline()
    if line:
        print(f"RAW: {line}")
        if line.startswith(b'p'):
            data = line.decode().strip().split()
            if len(data) > 2:
                pulses = [int(p) for p in data[2:]]
                print(f"PULSES: {pulses}")

s_ref.write(b'X21\n')
s_ref.close()
s_esp.close()
