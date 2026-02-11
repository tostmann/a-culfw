import serial
import time

s_ref = serial.Serial('/dev/ttyACM1', 38400, timeout=0.1)
s_esp = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

def get_rssi():
    s_ref.write(b'C34\n')
    time.sleep(0.05)
    return s_ref.read_all().decode().strip()

print(f"Initial RSSI: {get_rssi()}")
s_esp.write(b'V\n')
time.sleep(0.1)
print(f"ESP Version: {s_esp.read_all().decode().strip()}")

s_esp.write(b'y\n')
time.sleep(0.1)
print(f"ESP Status: {s_esp.read_all().decode().strip()}")

print("Sending P (Carrier for 2s)...")
s_esp.write(b'P\n')
for i in range(10):
    print(f"RSSI: {get_rssi()}")
    time.sleep(0.2)

print(f"Final RSSI: {get_rssi()}")
s_ref.close()
s_esp.close()
