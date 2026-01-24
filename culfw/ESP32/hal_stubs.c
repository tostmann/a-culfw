#include <stdint.h>

// --- UART Stubs ---
// Der ESP32 nutzt Arduino Serial (in main.cpp initialisiert).
// Die AVR-UART-Funktionen müssen wir abfangen.

void uart_init(uint8_t baud) { 
    // Passiert im setup() von main.cpp via Serial.begin()
}

void uart_task(void) {
    // Passiert im loop() von main.cpp
}

// --- USB Stubs ---
// culfw denkt, es sei ein CUL_V3 mit LUFA USB Stack.
// Wir faken die Init-Aufrufe.

void USB_Init(void) {}
void CDC_Task(void) {}

// --- Display Stubs ---
// Falls Logik versucht, auf das Display zu schreiben -> ignorieren.

uint8_t display_channel = 0;

void display_char(char c) {
    // Hier könnten wir später c an Serial.print leiten
}

void display_string(char *str) {
    // Stub
}

void display_nl(void) {
    // Stub
}
