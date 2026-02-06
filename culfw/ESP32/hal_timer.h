#ifndef __HAL_TIMER_H
#define __HAL_TIMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern volatile uint32_t ticks;
void hal_timer_init(void);
uint32_t hal_get_ticks(void);

void hal_enable_CC_timer_int(uint8_t instance, uint8_t enable);

uint32_t HAL_timer_get_reload_register(uint8_t instance);
void HAL_timer_set_reload_register(uint8_t instance, uint32_t value);
uint32_t HAL_timer_get_counter_value(uint8_t instance);
void HAL_timer_set_counter_value(uint8_t instance, uint32_t value);
void HAL_timer_reset_counter_value(uint8_t instance);

#ifdef __cplusplus
}
#endif

#endif
