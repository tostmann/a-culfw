import serial
import time

s_esp = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
s_ref = serial.Serial('/dev/ttyACM1', 38400, timeout=1)

def get_isr():
    s_esp.write(b'y\n')
    time.sleep(0.1)
    res = s_esp.read_all().decode()
    for line in res.split('\n'):
        if 'ISR:' in line:
            return line.strip()
    return "N/A"

print("Before:", get_isr())
for i in range(5):
    s_ref.write(b'is000000000FFF\n')
    time.sleep(0.5)
print("After:", get_isr())

s_esp.close()
s_ref.close()
