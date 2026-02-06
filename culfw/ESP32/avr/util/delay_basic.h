#ifndef _UTIL_DELAY_BASIC_H
#define _UTIL_DELAY_BASIC_H
#define _delay_loop_2(count) do { uint32_t __count = (count); while (__count--) { __asm__ volatile ("nop"); } } while (0)
#endif
