#ifndef _AVR_PGMSPACE_H
#define _AVR_PGMSPACE_H
#include <Arduino.h>
#define PROGMEM
#define PSTR(s) (s)
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#define pgm_read_word(addr) (*(const uint16_t *)(addr))
#define __LPM(addr) (*(const uint8_t *)(addr))
#define __LPM_word(addr) (*(const uint16_t *)(addr))
#endif
