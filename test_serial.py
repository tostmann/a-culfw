import serial
import time

try:
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
    ser.write(b'V\n')
    time.sleep(0.5)
    print(ser.read_all().decode(errors='ignore'))
    ser.close()
except Exception as e:
    print(e)
