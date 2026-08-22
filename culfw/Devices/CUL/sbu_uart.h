#ifndef _SBU_UART_H
#define _SBU_UART_H

#include <stdint.h>

#ifdef TTYSBU

// Global variables
extern uint8_t sbu_mode;

// Functions
void sbu_init(void);
void sbu_task(void);
void sbu_putc(char c);

#else

// Dummy implementation for Legacy builds
#define sbu_mode 0
#define sbu_init()
#define sbu_task()
#define sbu_putc(c)

#endif

#endif // _SBU_UART_H
