#ifndef _AVR_PGMSPACE_H_
#define _AVR_PGMSPACE_H_

#include <string.h>
#include <stdint.h>

// Auf dem ESP32 können wir Flash-Daten oft direkt lesen (Mapping).
// Daher definieren wir PROGMEM als leer.
#define PROGMEM
#define PGM_P const char *
#define PSTR(s) (s)

// Lese-Funktionen einfach auf Pointer-Dereferenzierung mappen
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#define pgm_read_word(addr) (*(const uint16_t *)(addr))
#define pgm_read_dword(addr) (*(const uint32_t *)(addr))

// String-Funktionen auf Standard-C Funktionen mappen
#define memcpy_P(dest, src, num) memcpy((dest), (src), (num))
#define strncmp_P(s1, s2, n) strncmp((s1), (s2), (n))
#define strcasecmp_P(s1, s2) strcasecmp((s1), (s2))
#define strcpy_P(dest, src) strcpy((dest), (src))
#define strlen_P(s) strlen((s))

#endif
