#ifndef _UTIL_PARITY_H_
#define _UTIL_PARITY_H_

#include <stdint.h>

// Die AVR-Funktion parity_even_bit gibt 1 zurück, wenn die Anzahl der gesetzten Bits ungerade ist.
// (Das ist das Paritätsbit, das man hinzufügen müsste, um "Even Parity" zu erreichen).
// Der GCC-Compiler hat dafür eine hocheffiziente Builtin-Funktion.

#define parity_even_bit(val) (__builtin_parity(val))

#endif
