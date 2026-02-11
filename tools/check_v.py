import serial
import time

try:
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=2)
    time.sleep(2)
    ser.reset_input_buffer()
    ser.write(b'\n') # wake up
    time.sleep(0.1)
    ser.write(b'V\n')
    time.sleep(0.5)
    print("V-Response:", ser.read_all().decode().strip())
    ser.write(b'y\n')
    time.sleep(0.5)
    print("y-Response:", ser.read_all().decode().strip())
    ser.close()
except Exception as e:
    print("Error:", e)
