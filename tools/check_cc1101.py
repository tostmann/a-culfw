import serial
import time
import sys

def check_cc1101(port):
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(2) # wait for reboot
        
        # Flush buffers
        ser.reset_input_buffer()
        
        print("Sending V...")
        ser.write(b'V\n')
        print("V Response:", ser.readline().decode().strip())
        
        print("Reading Register 0x30 (PARTNUM)...")
        ser.write(b'C30\n')
        print("C30 Response:", ser.readline().decode().strip())
        
        print("Reading Register 0x31 (VERSION)...")
        ser.write(b'C31\n')
        print("C31 Response:", ser.readline().decode().strip())
        
        print("Reading Register 0x00 (IOCFG2)...")
        ser.write(b'C00\n')
        print("C00 Response:", ser.readline().decode().strip())

        ser.close()
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    port = "/dev/ttyACM0"
    if len(sys.argv) > 1:
        port = sys.argv[1]
    check_cc1101(port)
