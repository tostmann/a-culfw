#include "delay.h"
#include "timebase.h"

#include <stdint.h>                     // for uint16_t, uint8_t

#include "util/delay_basic.h"           // for _delay_loop_2



void
my_delay_us( uint16_t d )
{
#if CULFW_CORE_HZ == 8000000UL
#if 1
  d<<=1;                // 4 cycles/loop, we are running 8MHz
  _delay_loop_2(d);    
#else
  TIMSK1 = 0;           // No interrupt if counter is reached
  TCNT1 = 0;            // The timer must be set up to run with 1MHz
  while (TCNT1<d) {};
#endif
#else
  // 4 cycles/loop at CULFW_CORE_HZ. 32 bit: above 8 MHz the loop count no longer
  // fits 16 bit for long delays (11.0592 MHz: from 23.7 ms on).
  uint32_t n = ((uint32_t)d * DELAY_LOOP_MUL + 128UL) >> 8;
  while (n > 0xffffUL) {
    _delay_loop_2(0);   // 0 = 65536 counts
    n -= 0x10000UL;
  }
  if (n)
    _delay_loop_2((uint16_t)n);
#endif
}

void
my_delay_ms( uint8_t d )
{
  while(d--) {
    my_delay_us( 1000 );
  }
}
