#!/root/.platformio/penv/bin/python3
import serial
import time
import sys

def listen():
    esp_port = '/dev/ttyACM0'
    ref_port = '/dev/ttyACM1'
    
    print(f"Opening {esp_port}...")
    esp = serial.Serial(esp_port, 115200, timeout=2)
    print(f"Opening {ref_port}...")
    ref = serial.Serial(ref_port, 115200, timeout=1)
    
    esp.dtr = True
    esp.rts = True
    ref.dtr = True
    ref.rts = True
    
    # Wait for ESP32 to boot and show Ready
    print("Waiting for ESP32 Ready...")
    start_time = time.time()
    found_ready = False
    while time.time() - start_time < 5:
        line = esp.readline().decode('utf-8', errors='ignore').strip()
        if line:
            print(f"[ESP32 Startup] {line}")
        if "Ready." in line:
            found_ready = True
            break
    
    if not found_ready:
        print("Warning: 'Ready.' not found, but continuing...")

    esp.reset_input_buffer()
    
    print("Setting ESP32 to X1F...")
    esp.write(b'X1F\r\n')
    time.sleep(0.5)
    
    # Flush output
    while esp.in_waiting > 0:
        line = esp.readline().decode('utf-8', errors='ignore').strip()
        print(f"[ESP32 Init] {line}")

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
    
    print("Requesting debug info (y)...")
    esp.write(b'y\r\n')
    time.sleep(1)
    while esp.in_waiting > 0:
        line = esp.readline().decode('utf-8', errors='ignore').strip()
        print(f"[ESP32 Debug] {line}")

    esp.close()
    ref.close()

if __name__ == "__main__":
    listen()
