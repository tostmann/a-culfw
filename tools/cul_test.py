#!/root/.platformio/penv/bin/python3
import serial
import sys
import time
import argparse

def main():
    parser = argparse.ArgumentParser(description='CULFW Serial Test Tool')
    parser.add_argument('port', help='Serial port (e.g. /dev/ttyACM1)')
    parser.add_argument('command', help='Command to send (e.g. V)')
    parser.add_argument('--timeout', type=float, default=2.0, help='Timeout in seconds')
    parser.add_argument('--baud', type=int, default=115200, help='Baudrate')
    
    args = parser.parse_args()

    try:
        # Initialisiere Port mit spezifischen ESP32-C3 Einstellungen (DTR/RTS)
        # Für den alten CUL868 (ATmega) schaden diese Einstellungen nicht.
        ser = serial.Serial(
            port=args.port,
            baudrate=args.baud,
            timeout=args.timeout,
            xonxoff=False,
            rtscts=False,
            dsrdtr=False
        )
        
        # Reset-Vermeidung/Handshake
        ser.dtr = True
        ser.rts = True
        time.sleep(0.1) 

        # Puffer leeren
        ser.reset_input_buffer()
        
        # Kommando senden (mit \r\n für culfw)
        cmd_bytes = (args.command + '\r\n').encode('utf-8')
        ser.write(cmd_bytes)
        
        # Antwort lesen (bis zum nächsten Prompt oder Timeout)
        # Wir lesen bis zu 1024 Bytes oder bis der Timeout greift
        response = b""
        start_time = time.time()
        
        while (time.time() - start_time) < args.timeout:
            if ser.in_waiting > 0:
                chunk = ser.read(ser.in_waiting)
                response += chunk
                # Wenn wir ein Newline nach dem Befehl sehen, könnte das die Antwort sein
                if len(response) > len(cmd_bytes) and b'\r\n' in response[len(cmd_bytes):]:
                    break
            time.sleep(0.05)

        if response:
            print(response.decode('utf-8', errors='ignore').strip())
        else:
            print("TIMEOUT")
            sys.exit(1)

    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == "__main__":
    main()
