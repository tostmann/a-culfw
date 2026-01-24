#ifndef _BOARD_H
#define _BOARD_H

#include <Arduino.h>

#define HAS_CC1100 1
// #define HAS_USB 1  <-- Auskommentiert lassen für nanoCUL Mode

#define TTY_BUFSIZE     128
#define RFR_BUFSIZE     128

// --- PIN MAPPING ---

// 1. Chip Select (CS)
// Wenn in platformio.ini nicht definiert, nutze Standard SS (meist GPIO 5 oder ähnlich)
#ifndef CC1100_CS_PIN
  #ifdef SS
    #define CC1100_CS_PIN SS
  #else
    #define CC1100_CS_PIN 5 // Fallback
  #endif
#endif

// 2. SPI Pins (SCK, MISO, MOSI)
// Wir nutzen die Standard-Definitionen des Boards (pins_arduino.h)
#ifndef CC1100_SCK_PIN
  #ifdef SCK
    #define CC1100_SCK_PIN SCK
  #else
    #define CC1100_SCK_PIN 18
  #endif
#endif

#ifndef CC1100_MISO_PIN
  #ifdef MISO
    #define CC1100_MISO_PIN MISO
  #else
    #define CC1100_MISO_PIN 19
  #endif
#endif

#ifndef CC1100_MOSI_PIN
  #ifdef MOSI
    #define CC1100_MOSI_PIN MOSI
  #else
    #define CC1100_MOSI_PIN 23
  #endif
#endif

// 3. Konfigurierbare Pins (Müssen via platformio.ini kommen, sonst Standard)
#ifndef CC1100_GDO0_PIN
  #define CC1100_GDO0_PIN 4
#endif

#ifndef CC1100_GDO2_PIN
  #define CC1100_GDO2_PIN 2
#endif

#ifndef LED_PIN
  #ifdef LED_BUILTIN
    #define LED_PIN LED_BUILTIN
  #else
    #define LED_PIN 22
  #endif
#endif

// --- Fake Ports für AVR Kompatibilität ---
#define CC1100_CS_DDR   DDRB
#define CC1100_CS_PORT  PORTB
#define CC1100_IN_PORT  PINB
#define LED_PORT        PORTB
#define LED_DDR         DDRB

#define CC1100_INT      4 
#define ISC00           0
#define ISC01           1

// SlowRF Config
#define RCV_BUCKETS     2 

// Mapping für rf_receive
#define CC1100_IN_DDR   DDRB
#define CC1100_IN_PORT  PORTB
#define CC1100_IN_PIN   CC1100_GDO0_PIN

#define CC1100_OUT_DDR  DDRB
#define CC1100_OUT_PORT PORTB
#define CC1100_OUT_PIN  CC1100_GDO2_PIN

#define CC1100_EICR     EICRA
#define CC1100_ISC      0

// Macros auf Arduino mappen
#define LED_ON()        digitalWrite(LED_PIN, HIGH)
#define LED_OFF()       digitalWrite(LED_PIN, LOW)
#define LED_TOGGLE()    digitalWrite(LED_PIN, !digitalRead(LED_PIN))

#define CC1100_INT_VECT  cc1100_interrupt_routine 
#define BOARD_ID_STR "ESP32_Native"

void spi_init(void);

#endif
