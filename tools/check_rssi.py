import serial
import time

def check_rssi():
    try:
        esp = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
        ref = serial.Serial('/dev/ttyACM1', 115200, timeout=1)
        time.sleep(2)
        
        esp.write(b'X21\r\n') # RX mode
        time.sleep(0.5)
        
        print("Reading baseline RSSI (XIAO C6)...")
        for _ in range(5):
            esp.write(b'C34\r\n')
            time.sleep(0.5)
            line = esp.read_all().decode().strip()
            print(f"RSSI: '{line}'")
            
        print("\nSending from Ref CUL and reading RSSI (XIAO C6)...")
        for _ in range(5):
            ref.write(b'F12340111\r\n') # Send FS20
            time.sleep(0.05)
            esp.write(b'C34\r\n')
            time.sleep(0.1)
            print(f"RSSI during send: {esp.read_all().decode().strip()}")
            time.sleep(0.5)

        esp.close()
        ref.close()
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    check_rssi()
