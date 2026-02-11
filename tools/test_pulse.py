import serial
import time
import sys

try:
    s_ref = serial.Serial('/dev/ttyACM1', 38400, timeout=0.1)
    s_esp = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
    
    # Flush
    s_ref.read_all()
    s_esp.read_all()
    
    print("Starting pulse on ESP32...")
    s_esp.write(b'P\n')
    
    start_time = time.time()
    while time.time() - start_time < 4:
        s_ref.write(b'C34\n')
        time.sleep(0.2)
        res = s_ref.read_all().decode().strip()
        if res:
            print(f"Ref CUL RSSI: {res}")
        
        esp_res = s_esp.read_all().decode().strip()
        if esp_res:
            print(f"ESP32: {esp_res}")
            
    s_ref.close()
    s_esp.close()
except Exception as e:
    print(f"Error: {e}")
