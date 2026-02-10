import serial
import time

esp_port = '/dev/ttyACM0'

try:
    esp = serial.Serial(esp_port, 115200, timeout=1)
except Exception as e:
    print(f"Error opening serial: {e}")
    exit(1)

esp.write(b'C99\r\n')
time.sleep(0.5)
while esp.in_waiting:
    print(esp.readline().decode('ascii', errors='replace').strip())

esp.close()
