import serial
import time
import sys

s_ref = serial.Serial('/dev/ttyACM1', 38400, timeout=0.1)
s_esp = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

s_ref.write(b'X21\n')
time.sleep(0.1)
s_ref.read_all()

print("Listening on REF CUL. Sending F12340111 from ESP32...")
s_esp.write(b'F12340111\n')

start = time.time()
while time.time() - start < 5:
    line = s_ref.readline()
    if line:
        print(f"REF GOT: {line.decode().strip()}")
    esp_line = s_esp.readline()
    if esp_line:
        print(f"ESP GOT: {esp_line.decode().strip()}")

s_ref.close()
s_esp.close()
