#ifndef _AVR_INTERRUPT_H_
#define _AVR_INTERRUPT_H_

#include <Arduino.h>

// Globale Interrupts steuern
#define cli() noInterrupts()
#define sei() interrupts()

// ISR Makro: In AVR definiert man so Interrupt-Funktionen.
// Auf ESP32 funktioniert das anders (attachInterrupt), aber damit der Code
// kompiliert, definieren wir ISR als normale Funktion.
// ACHTUNG: Die Logik dahinter muss später in main.cpp manuell verknüpft werden!
#define ISR(vector, ...) void vector(void)

// Dummy Vektoren (damit der Name existiert)
#define TIMER1_COMPA_vect  Fake_Timer1_ISR

#endif
