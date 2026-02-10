#include <Arduino.h>

#if !ARDUINO_USB_CDC_ON_BOOT
// Wenn CDC_ON_BOOT nicht gesetzt ist, müssen wir USBSerial nehmen
#define MySerial USBSerial
#else
#define MySerial Serial
#endif

void setup() {
    MySerial.begin(115200);
    pinMode(21, OUTPUT);
}

void loop() {
    digitalWrite(21, HIGH);
    delay(100);
    digitalWrite(21, LOW);
    delay(100);
    MySerial.println("HELLO_XIAO_USB");
}
