import serial
import time

s_ref = serial.Serial('/dev/ttyACM1', 38400, timeout=1)
s_esp = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

# REF CUL in normal mode, report everything
s_ref.write(b'X21\n')
time.sleep(0.5)
s_ref.read_all()

print("Sending FS20 packet from ESP32...")
s_esp.write(b'F12340111\n')

start = time.time()
found = False
while time.time() - start < 5:
    line = s_ref.readline()
    if line:
        print(f"REF: {line.decode().strip()}")
        if b'F12340111' in line:
            found = True
            break

if found:
    print("SUCCESS: FS20 packet received by REF CUL!")
else:
    print("FAILED: No FS20 packet received.")

s_ref.close()
s_esp.close()
