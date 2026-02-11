import serial
import time
import threading

def sender():
    try:
        s_ref = serial.Serial('/dev/ttyACM1', 38400)
        for i in range(10):
            print(f"Sending FS20 packet {i}...")
            s_ref.write(b'F12340111\n')
            time.sleep(0.5)
        s_ref.close()
    except Exception as e:
        print(f"Sender error: {e}")

t = threading.Thread(target=sender)

try:
    s = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
    s.write(b'X21\n')
    time.sleep(0.5)
    s.read_all()

    print("Starting sender thread...")
    t.start()

    start_time = time.time()
    while time.time() - start_time < 10:
        line = s.readline()
        if line:
            print(f"GOT: {line.decode().strip()}")
        else:
            # Send y periodically to see status
            s.write(b'y\n')
            status = s.readline()
            if status:
                print(f"STATUS: {status.decode().strip()}")
            time.sleep(0.5)

    t.join()
    s.close()
except Exception as e:
    print(f"Receiver error: {e}")
