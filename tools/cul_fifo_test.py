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
        time.sleep(1)
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
                        print(f"[{self.name} RX] {line}")
                        self.received_lines.append(line)
                    buffer = b""
                elif char != b'\r':
                    buffer += char
            else:
                time.sleep(0.01)

    def send(self, cmd):
        print(f"[{self.name} TX] {cmd}")
        self.ser.write((cmd + '\r\n').encode('utf-8'))

    def wait_for_line(self, pattern, timeout=5):
        start = time.time()
        while (time.time() - start) < timeout:
            for i in range(len(self.received_lines)):
                line = self.received_lines[i]
                if pattern in line:
                    return self.received_lines.pop(i)
            time.sleep(0.1)
        return None

    def close(self):
        self.running = False
        self.ser.close()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--esp', default='/dev/ttyACM0')
    parser.add_argument('--ref', default='/dev/ttyACM1')
    args = parser.parse_args()

    try:
        esp = CUL(args.esp, "ESP32")
        ref = CUL(args.ref, "REF")

        print("\n--- Testing MAX! (Moritz) FIFO Mode ---")
        
        # Enable MAX! Receiver mode on both
        esp.send('Zr')
        ref.send('Zr')
        time.sleep(1)

        # TEST 1: REF sends to ESP32
        # Command: Zs <len><msgcnt><flag><type><src><dst><group><payload>
        # Len 0x0B (11), msgcnt 01, flag 00, type 40 (SetTemp), src 112233, dst 445566, group 00, payload 00
        test_packet = "0B0100401122334455660000"
        print(f"\nREF sending MAX! packet: {test_packet}")
        ref.send('Zs' + test_packet)
        
        match = esp.wait_for_line("Z" + test_packet, timeout=10)
        if match:
            print("SUCCESS: ESP32 received the MAX! packet.")
        else:
            print("FAILED: ESP32 did not receive the MAX! packet.")

        # TEST 2: ESP32 sends to REF
        print(f"\nESP32 sending MAX! packet: {test_packet}")
        esp.send('Zs' + test_packet)
        
        match = ref.wait_for_line("Z" + test_packet, timeout=10)
        if match:
            print("SUCCESS: REF received the MAX! packet.")
        else:
            print("FAILED: REF did not receive the MAX! packet.")

    except KeyboardInterrupt:
        pass
    finally:
        if 'esp' in locals(): esp.close()
        if 'ref' in locals(): ref.close()

if __name__ == "__main__":
    main()
