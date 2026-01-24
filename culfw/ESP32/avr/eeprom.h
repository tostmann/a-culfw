#ifndef _AVR_EEPROM_H_
#define _AVR_EEPROM_H_

#include <stddef.h>
#include <stdint.h>

#define EEMEM
#define E2END 0x0FFF

// Prototypen für die CLIB
uint8_t eeprom_read_byte (const uint8_t *__p);
uint16_t eeprom_read_word (const uint16_t *__p);
void eeprom_write_byte (uint8_t *__p, uint8_t __value);
void eeprom_write_word (uint16_t *__p, uint16_t __value);
void eeprom_read_block (void *__dst, const void *__src, size_t __n);
void eeprom_write_block (const void *__src, void *__dst, size_t __n);

// Dummies
static inline int eeprom_is_ready (void) { return 1; }
static inline void eeprom_busy_wait (void) {}

#endif
