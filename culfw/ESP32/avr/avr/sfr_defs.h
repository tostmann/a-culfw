#ifndef _AVR_SFR_DEFS_H
#define _AVR_SFR_DEFS_H

#ifdef ESP32
#include <Arduino.h>
#define bit_is_set(sfr, bit) (digitalRead(sfr))
#define bit_is_clear(sfr, bit) (!digitalRead(sfr))
#else
#define bit_is_set(sfr, bit) (sfr & (1 << bit))
#define bit_is_clear(sfr, bit) (!(sfr & (1 << bit)))
#endif
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))
#endif
