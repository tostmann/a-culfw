#!/root/.platformio/penv/bin/python3
import serial
import time
import sys

def listen():
    esp_port = '/dev/ttyACM0'
    ref_port = '/dev/ttyACM1'
    
    esp = serial.Serial(esp_port, 115200, timeout=1)
    ref = serial.Serial(ref_port, 115200, timeout=1)
    
    esp.dtr = True
    esp.rts = True
    ref.dtr = True
    ref.rts = True
    time.sleep(1)
    
    esp.reset_input_buffer()
    
    print("Setting ESP32 to X07...")
    esp.write(b'X07\r\n')
    time.sleep(0.5)
    
    print("Sending FS20 from REF...")
    for i in range(5):
        print(f"Send {i+1}/5")
        ref.write(b'F12340111\r\n')
        time.sleep(0.5)
        
        # Check ESP output
        while esp.in_waiting > 0:
            line = esp.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(f"[ESP32] {line}")
    
    esp.write(b'y\r\n')
    time.sleep(0.5)
    while esp.in_waiting > 0:
        line = esp.readline().decode('utf-8', errors='ignore').strip()
        print(f"[ESP32 Debug] {line}")

    esp.close()
    ref.close()

if __name__ == "__main__":
    listen()
