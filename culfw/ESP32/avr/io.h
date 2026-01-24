#ifndef _AVR_IO_H_
#define _AVR_IO_H_

#include <stdint.h>
#include <Arduino.h> 

// --- Makros ---
#ifndef _BV
#define _BV(b) (1UL << (b))
#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Interrupts
#define cli() noInterrupts()
#define sei() interrupts()

// --- Fake AVR Register (deklaration) ---
// Diese Variablen existieren nur, damit der originale C-Code kompiliert.
// Sie haben KEINE Auswirkung auf die Hardware!
extern volatile uint8_t SPDR;
extern volatile uint8_t SPSR;
extern volatile uint8_t EIMSK;
extern volatile uint8_t EICRA;
extern volatile uint8_t DDRB;
extern volatile uint8_t PORTB;
extern volatile uint8_t PINB;
extern volatile uint8_t SREG; // Status Register Dummy

extern volatile uint8_t TIMSK0;
// Timer 1 (16-Bit) Simulation
extern volatile uint8_t  TIMSK1;
extern volatile uint8_t  TIFR1;
extern volatile uint16_t TCNT1;
extern volatile uint16_t OCR1A;

// Timer Interrupt Flags
#define OCF1A   1
#define OCIE1A  1

// Bit Definitionen
#define SPIF 7
#define SPE  6
#define MSTR 4
#define SPR0 0
#define SPR1 1
#define SPI2X 0 // Double speed ignorieren

#define ISC00 0
#define ISC01 1

#endif
