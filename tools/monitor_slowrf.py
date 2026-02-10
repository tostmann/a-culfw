import serial
import time
import sys

port = '/dev/ttyACM0'
baud = 115200

try:
    ser = serial.Serial(port, baud, timeout=1)
    time.sleep(2) # wait for reset
    
    # Check version
    ser.write(b'V\n')
    print("Sent: V")
    time.sleep(0.5)
    print("Response V:", ser.read_all().decode(errors='ignore').strip())
    
    # Enable monitor mode with bits
    ser.write(b'X3F\n')
    print("Sent: X3F")
    time.sleep(0.5)
    
    # Send 'y' for debug
    ser.write(b'y\n')
    print("Sent: y")
    time.sleep(0.5)
    print("Response y:", ser.read_all().decode(errors='ignore').strip())

    print("Monitoring for 15 seconds... (Press Ctrl+C to stop)")
    start_time = time.time()
    while time.time() - start_time < 15:
        if ser.in_available():
            line = ser.read(ser.in_waiting).decode(errors='ignore')
            sys.stdout.write(line)
            sys.stdout.flush()
        time.sleep(0.1)

except Exception as e:
    print(f"Error: {e}")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
