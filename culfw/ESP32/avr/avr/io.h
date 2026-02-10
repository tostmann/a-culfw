#ifndef _AVR_IO_H
#define _AVR_IO_H
#pragma GCC system_header
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
extern uint8_t SREG;
#ifdef __cplusplus
}
#endif
#ifndef _BV
#define _BV(bit) (1UL << (bit))
#endif

// Dummy registers for compatibility
extern uint8_t TIMSK0;

#endif
