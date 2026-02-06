#ifndef _AVR_EEPROM_H
#define _AVR_EEPROM_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
uint8_t eeprom_read_byte(uint8_t *addr);
void eeprom_write_byte(uint8_t *addr, uint8_t value);
#ifdef __cplusplus
}
#endif

// On ESP32 we use the functions from fncollection.c
// but we need to declare them if they are used elsewhere.
#ifdef __cplusplus
extern "C" {
#endif
uint8_t erb(uint8_t *p);
void ewb(uint8_t *p, uint8_t v);
#ifdef __cplusplus
}
#endif

#define eeprom_busy_wait()
#define eeprom_read_word(addr) ( (uint16_t)erb((uint8_t*)(addr)) | ((uint16_t)erb((uint8_t*)(addr)+1) << 8) )

#endif
