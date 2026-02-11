import serial
import time

def get_freq(s, name):
    s.write(b'C0D\n')
    time.sleep(0.1)
    f2 = s.read_all().decode().strip()
    s.write(b'C0E\n')
    time.sleep(0.1)
    f1 = s.read_all().decode().strip()
    s.write(b'C0F\n')
    time.sleep(0.1)
    f0 = s.read_all().decode().strip()
    print(f"{name} Freq: {f2}, {f1}, {f0}")

s_ref = serial.Serial('/dev/ttyACM1', 38400, timeout=1)
s_esp = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

get_freq(s_ref, "REF")
get_freq(s_esp, "ESP")

s_ref.close()
s_esp.close()
