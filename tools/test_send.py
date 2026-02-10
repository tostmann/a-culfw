import serial
import time
import sys

# ESP32
esp_port = '/dev/ttyACM0'
# CUL (Reference)
cul_port = '/dev/ttyACM1'

try:
    esp = serial.Serial(esp_port, 115200, timeout=1)
    cul = serial.Serial(cul_port, 38400, timeout=1)
except Exception as e:
    print(f"Error opening serial: {e}")
    sys.exit(1)

print("Setting up ESP32...")
esp.write(b'X3F\r\n') # Monitor mode + Binary timings + Known packets
time.sleep(0.5)

print("Sending FS20 from CUL...")
# FS20 command: F123401 (Housecode 1234, Address 01, Command on)
# Raw FS20 for CUL: F123401
cul.write(b'F123401\r\n')
time.sleep(0.5)

print("Monitoring ESP32 for 5 seconds...")
start_time = time.time()
while time.time() - start_time < 5:
    if esp.in_waiting:
        line = esp.readline().decode('ascii', errors='replace').strip()
        if line:
            print(f"[ESP32] {line}")
    time.sleep(0.01)

esp.close()
cul.close()
