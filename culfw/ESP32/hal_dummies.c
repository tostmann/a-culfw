#include <stdint.h>

// Speicherplatz für die Fake-Register bereitstellen
volatile uint8_t SPDR = 0;
volatile uint8_t SPSR = 0;
volatile uint8_t EIMSK = 0;
volatile uint8_t EICRA = 0;
volatile uint8_t DDRB = 0;
volatile uint8_t PORTB = 0;
volatile uint8_t PINB = 0;
volatile uint8_t SREG = 0;
volatile uint8_t TIMSK0 = 0;
// Timer 1 Dummies
volatile uint8_t  TIMSK1 = 0;
volatile uint8_t  TIFR1  = 0;
volatile uint16_t TCNT1  = 0;
volatile uint16_t OCR1A  = 0;
