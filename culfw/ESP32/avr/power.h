#ifndef _AVR_POWER_H_
#define _AVR_POWER_H_
#include <stdint.h>
typedef enum { clock_div_1 = 0 } clock_div_t;
static inline void clock_prescale_set(clock_div_t x) { (void)x; }
#endif
