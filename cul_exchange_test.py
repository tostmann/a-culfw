#!/root/.platformio/penv/bin/python3
import serial
import time
import sys
import argparse
import threading

class CUL:
    def __init__(self, port, name, baud=115200, timeout=1):
        self.port = port
        self.name = name
        self.ser = serial.Serial(port, baud, timeout=timeout)
        self.ser.dtr = True
        self.ser.rts = True
        time.sleep(0.5)
        self.ser.reset_input_buffer()
        self.received_lines = []
        self.running = True
        self.thread = threading.Thread(target=self._listen, daemon=True)
        self.thread.start()

    def _listen(self):
        buffer = b""
        while self.running:
            if self.ser.in_waiting > 0:
                char = self.ser.read(1)
                if char == b'\n':
                    line = buffer.decode('utf-8', errors='ignore').strip()
                    if line:
                        self.received_lines.append(line)
                    buffer = b""
                elif char != b'\r':
                    buffer += char
            else:
                time.sleep(0.01)

    def send(self, cmd):
        print(f"[{self.name}] Sending: {cmd}")
        self.ser.write((cmd + '\r\n').encode('utf-8'))

    def wait_for_line(self, pattern, timeout=2):
        start = time.time()
        while (time.time() - start) < timeout:
            for i, line in enumerate(self.received_lines):
                if pattern in line:
                    return self.received_lines.pop(i)
            time.sleep(0.1)
        return None

    def close(self):
        self.running = False
        self.ser.close()

def test_communication(sender, receiver, test_name, cmd, expected_pattern, timeout=5):
    print(f"\n--- Test: {test_name} ---")
    receiver.received_lines.clear()
    sender.send(cmd)
    
    match = receiver.wait_for_line(expected_pattern, timeout)
    if match:
        print(f"SUCCESS: {receiver.name} received '{match}'")
        return True
    else:
        print(f"FAILED: {receiver.name} did not receive expected pattern '{expected_pattern}'")
        if receiver.received_lines:
            print(f"Last received lines on {receiver.name}: {receiver.received_lines[-5:]}")
        return False

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--esp', default='/dev/ttyACM0')
    parser.add_argument('--ref', default='/dev/ttyACM1')
    args = parser.parse_args()

    print(f"Initializing ESP32 on {args.esp} and Reference CUL on {args.ref}")
    try:
        esp = CUL(args.esp, "ESP32")
        ref = CUL(args.ref, "REF")

        # Basic Sanity
        esp.send('V')
        ref.send('V')
        time.sleep(0.5)
        print(f"ESP32 Version: {esp.received_lines.pop(0) if esp.received_lines else 'TIMEOUT'}")
        print(f"REF Version: {ref.received_lines.pop(0) if ref.received_lines else 'TIMEOUT'}")

        # Initial Setup for both: 868.3 MHz, X21 (Report all RF)
        for dev in [esp, ref]:
            dev.send('X21') # Receiver mode
            dev.send('t01') # Enable timestamps if needed (optional)

        # TEST 1: ESP32 sends, REF receives (FS20)
        # F + 4 hex (housecode) + 2 hex (addr) + 2 hex (cmd)
        # We use a dummy FS20 command
        test_communication(esp, ref, "ESP -> REF (FS20)", "F12340111", "F12340111")

        # TEST 2: REF sends, ESP32 receives (FS20)
        test_communication(ref, esp, "REF -> ESP (FS20)", "F56780222", "F56780222")

        # TEST 3: SlowRF / Intertechno (if enabled)
        # Sending 'i' command
        test_communication(esp, ref, "ESP -> REF (Intertechno)", "is000000000FFF", "i000000")
        test_communication(ref, esp, "REF -> ESP (Intertechno)", "isAAAAAAAAAAAA", "iAAAAAA")

    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if 'esp' in locals(): esp.close()
        if 'ref' in locals(): ref.close()

if __name__ == "__main__":
    main()
