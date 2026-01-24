#ifndef _AVR_SFR_DEFS_H_
#define _AVR_SFR_DEFS_H_

#include <stdint.h>

// Bit Values (1 << x)
#ifndef _BV
#define _BV(bit) (1UL << (bit))
#endif

// Prüfen ob ein Bit gesetzt ist
#ifndef bit_is_set
#define bit_is_set(sfr, bit) ((sfr) & _BV(bit))
#endif

// Prüfen ob ein Bit gelöscht ist
#ifndef bit_is_clear
#define bit_is_clear(sfr, bit) (!((sfr) & _BV(bit)))
#endif

// Warten bis Bit gesetzt ist (Polling Loop)
#ifndef loop_until_bit_is_set
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
#endif

// Warten bis Bit gelöscht ist
#ifndef loop_until_bit_is_clear
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))
#endif

#endif
