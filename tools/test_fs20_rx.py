import serial
import time
import threading
import sys

def monitor_esp32():
    try:
        ser = serial.Serial('/dev/ttyACM0', 115200, timeout=0.1)
        time.sleep(2)
        ser.write(b'V\n')
        ser.write(b'X3F\n')
        ser.write(b'y\n')
        print("ESP32 Monitor started...")
        end_time = time.time() + 10
        while time.time() < end_time:
            if ser.in_waiting:
                data = ser.read(ser.in_waiting).decode(errors='ignore')
                sys.stdout.write(data)
                sys.stdout.flush()
            time.sleep(0.01)
        ser.close()
    except Exception as e:
        print(f"ESP32 Monitor Error: {e}")

def send_fs20_ref():
    try:
        time.sleep(3) # Wait for monitor to be ready
        ser = serial.Serial('/dev/ttyACM1', 38400, timeout=1) # Ref CUL often 38400 or 9600
        print("\nRef CUL: Sending FS20 command...")
        ser.write(b'F112201\n')
        time.sleep(1)
        print("Ref CUL: Sent F112201")
        ser.close()
    except Exception as e:
        print(f"Ref CUL Error: {e}")

t1 = threading.Thread(target=monitor_esp32)
t2 = threading.Thread(target=send_fs20_ref)

t1.start()
t2.start()

t1.join()
t2.join()
