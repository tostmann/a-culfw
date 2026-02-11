import serial
import time

# ESP32 C6 on /dev/ttyACM0
# REF CUL on /dev/ttyACM1

try:
    s_esp = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
    s_ref = serial.Serial('/dev/ttyACM1', 38400, timeout=1)

    print("Setting ESP32 to Intertechno mode (it)...")
    s_esp.write(b'it\n')
    time.sleep(1)
    print(f"ESP Resp: {s_esp.read_all().decode().strip()}")

    print("Sending Intertechno (is000000000FFF) from REF CUL...")
    s_ref.write(b'is000000000FFF\n')
    time.sleep(0.5)
    print(f"REF Resp: {s_ref.read_all().decode().strip()}")

    print("Listening for 10 seconds on ESP32...")
    start = time.time()
    while time.time() - start < 10:
        line = s_esp.readline()
        if line:
            print(f"ESP RX: {line.decode().strip()}")
        
        # Also check debug if nothing comes
        if time.time() - start > 5 and not line:
             s_esp.write(b'y\n')
             debug = s_esp.readline()
             if debug:
                 print(f"DEBUG: {debug.decode().strip()}")

except Exception as e:
    print(f"Error: {e}")
finally:
    s_esp.close()
    s_ref.close()
