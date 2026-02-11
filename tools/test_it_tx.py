import serial
import time
import sys

s_ref = serial.Serial('/dev/ttyACM1', 38400, timeout=1)
s_esp = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

# Put REF CUL in normal mode
s_ref.write(b'X21\n')
time.sleep(0.5)
s_ref.read_all()

# Intertechno command example: is000000000FFF (on)
# IT commands are like is000000000FFF
print("REF CUL in X21. Triggering Intertechno (is000000000FFF) on ESP32...")
s_esp.write(b'is000000000FFF\n')

# Listen for 5 seconds
start = time.time()
while time.time() - start < 5:
    line = s_ref.readline()
    if line:
        print(f"REF: {line.decode().strip()}")
    esp_line = s_esp.readline()
    if esp_line:
        print(f"ESP: {esp_line.decode().strip()}")

print("\nDone.")
s_ref.close()
s_esp.close()
