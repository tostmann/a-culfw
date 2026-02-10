
import serial
import time

def test_serial():
    print("Testing Serial on /dev/ttyACM0...")
    try:
        ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
        # Toggle DTR/RTS to force reset
        ser.dtr = False
        ser.rts = False
        time.sleep(0.1)
        ser.dtr = True
        ser.rts = True
        time.sleep(1)
        
        start_time = time.time()
        while time.time() - start_time < 10:
            line = ser.readline()
            if line:
                print(f"Received: {line.decode('ascii', errors='replace').strip()}")
            if b"Heartbeat" in line or b"Ready" in line:
                print("Found expected output!")
                return True
    except Exception as e:
        print(f"Error: {e}")
    return False

if __name__ == "__main__":
    test_serial()
