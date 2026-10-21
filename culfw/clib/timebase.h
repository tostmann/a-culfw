#ifndef _TIMEBASE_H
#define _TIMEBASE_H

/*
 * SlowRF time base.
 *
 * The clib measures and generates SlowRF pulses in microseconds of an 8 MHz core:
 * Timer1 runs at core/8 = 1 MHz, pulse lengths are kept in 16 us units
 * (TCNT1 >> 4) and my_delay_us() burns 8 cycles per microsecond.
 *
 * A board whose core really runs at another frequency defines CULFW_CORE_HZ in its
 * board.h (SHC / SHC2: 11.0592 MHz crystal). The macros below then convert between
 * Timer1 ticks and microseconds, so every constant and every pulse value keeps its
 * 8 MHz meaning and still fits the same variable widths. Without that define all of
 * this reduces to the original 8 MHz code.
 *
 * CULFW_CORE_HZ is the frequency the core actually runs at - after any
 * clock_prescale_set(). F_CPU does not always say that: CUL-Arduino and I2CCUL
 * build with F_CPU = 16 MHz and prescale to 8 MHz at runtime.
 */

#include <stdint.h>
#include "board.h"

#ifndef CULFW_CORE_HZ
#define CULFW_CORE_HZ   8000000UL
#endif

#if CULFW_CORE_HZ == 8000000UL

#define US2TICK(us)     (us)
#define TICK2US16(t)    ((t) >> 4)

#else

#define T1_HZ           (CULFW_CORE_HZ / 8ULL)
/* us -> Timer1 ticks, 8.8 fixed point (11.0592 MHz: 354/256 for 1.3824) */
#define US2TICK_MUL     ((uint32_t)((T1_HZ * 256ULL + 500000ULL) / 1000000ULL))
#define US2TICK(us)     ((uint16_t)(((uint32_t)(uint16_t)(us) * US2TICK_MUL + 128UL) >> 8))
/* Timer1 ticks -> 16 us units, 0.16 fixed point (11.0592 MHz: 2963) */
#define TICK2US16_MUL   ((uint32_t)((1000000ULL * 4096ULL + T1_HZ / 2ULL) / T1_HZ))
#define TICK2US16(t)    ((uint16_t)(((uint32_t)(t) * TICK2US16_MUL) >> 16))
/* my_delay_us(): _delay_loop_2 counts (4 cycles each) per us, 8.8 fixed point (11.0592 MHz: 708) */
#define DELAY_LOOP_MUL  ((uint32_t)((CULFW_CORE_HZ / 4ULL * 256ULL + 500000ULL) / 1000000ULL))

#endif

#endif
