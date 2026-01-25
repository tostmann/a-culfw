#include <Arduino.h>

extern "C" {

void uart_putchar(char c) {
    Serial.write(c);
}

void uart_putstr(char *s) {
    while(*s) {
        Serial.write(*s++);
    }
}

void uart_init(uint8_t baud) {
    // In main.cpp erledigt
}

void cdc_flush(void) {
    Serial.flush();
}

void USB_Flush(void) {
    Serial.flush();
}

void uart_task(void) {
    // Dummy
}

} // end extern "C"
