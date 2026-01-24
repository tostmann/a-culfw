#include <stdint.h>
#include <string.h>
#include "avr/eeprom.h"

// Virtuelles EEPROM im RAM
static uint8_t sim_eeprom[4096];

uint8_t eeprom_read_byte(const uint8_t *addr) {
    uintptr_t offset = (uintptr_t)addr;
    if (offset < sizeof(sim_eeprom)) {
        return sim_eeprom[offset];
    }
    return 0xFF;
}

void eeprom_write_byte(uint8_t *addr, uint8_t value) {
    uintptr_t offset = (uintptr_t)addr;
    if (offset < sizeof(sim_eeprom)) {
        sim_eeprom[offset] = value;
    }
}

uint16_t eeprom_read_word(const uint16_t *addr) {
    uintptr_t offset = (uintptr_t)addr;
    if (offset + 1 < sizeof(sim_eeprom)) {
        return sim_eeprom[offset] | (sim_eeprom[offset+1] << 8);
    }
    return 0xFFFF;
}

void eeprom_write_word(uint16_t *addr, uint16_t value) {
    uintptr_t offset = (uintptr_t)addr;
    if (offset + 1 < sizeof(sim_eeprom)) {
        sim_eeprom[offset] = value & 0xFF;
        sim_eeprom[offset+1] = (value >> 8) & 0xFF;
    }
}

void eeprom_read_block(void *dst, const void *src, size_t n) {
    uintptr_t offset = (uintptr_t)src;
    if (offset + n <= sizeof(sim_eeprom)) {
        memcpy(dst, &sim_eeprom[offset], n);
    }
}

void eeprom_write_block(const void *src, void *dst, size_t n) {
    uintptr_t offset = (uintptr_t)dst;
    if (offset + n <= sizeof(sim_eeprom)) {
        memcpy(&sim_eeprom[offset], src, n);
    }
}
