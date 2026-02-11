import serial
import time

def test_xiao_c6():
    try:
        ser = serial.Serial('/dev/ttyACM0', 115200, timeout=2)
        time.sleep(2) # Wait for boot
        
        commands = [b'V\r\n', b'C30\r\n', b'C31\r\n', b'C02\r\n', b'C00\r\n']
        for cmd in commands:
            ser.write(cmd)
            time.sleep(0.2)
            while ser.in_waiting:
                line = ser.readline().decode('ascii', errors='replace').strip()
                print(f"Cmd: {cmd.strip().decode()} -> {line}")
        
        ser.close()
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    test_xiao_c6()
