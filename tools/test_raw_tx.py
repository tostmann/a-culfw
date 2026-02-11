import serial
import time

s_ref = serial.Serial('/dev/ttyACM1', 38400, timeout=0.1)
s_esp = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

# Put REF CUL in raw mode
s_ref.write(b'X67\n')
time.sleep(0.5)
s_ref.read_all()

print("REF CUL in X67 mode. Sending P from ESP32...")
s_esp.write(b'P\n')

start = time.time()
while time.time() - start < 2:
    line = s_ref.readline()
    if line:
        print(f"REF: {line.decode().strip()}")
    esp_line = s_esp.readline()
    if esp_line:
        print(f"ESP: {esp_line.decode().strip()}")

s_ref.write(b'X21\n')
s_ref.close()
s_esp.close()
