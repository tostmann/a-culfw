import serial
import time

def verify_spi():
    try:
        ser = serial.Serial('/dev/ttyACM0', 115200, timeout=2)
        time.sleep(1)
        
        # Read initial
        ser.write(b'C02\r\n')
        time.sleep(0.1)
        print(f"Initial C02: {ser.read_all().decode().strip()}")
        
        # Write new value 0x55 using Cw command
        ser.write(b'Cw0255\r\n')
        time.sleep(0.1)
        ser.read_all() # Clear buffer
        
        # Read back
        ser.write(b'C02\r\n')
        time.sleep(0.1)
        print(f"After write C02: {ser.read_all().decode().strip()}")
        
        ser.close()
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    verify_spi()
