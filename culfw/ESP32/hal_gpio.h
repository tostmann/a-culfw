#ifndef __HAL_GPIO_H
#define __HAL_GPIO_H

#include <Arduino.h>
#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INIT_MODE_OUT_CS_IN    0
#define INIT_MODE_IN_CS_IN     1

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef enum {
    LED0 = 0,
    LED_COUNT
} LED_List;

typedef enum {
    LED_off = 0,
    LED_on
} LED_State;

typedef enum {
    CC_Pin_Out = 0,
    CC_Pin_CS,
    CC_Pin_In
} CC_PIN;

void HAL_LED_Init(void);
void HAL_LED_Set(LED_List led, LED_State state);
void HAL_LED_Toggle(LED_List led);

void hal_CC_GDO_init(uint8_t cc_num, uint8_t mode);
void hal_enable_CC_GDOin_int(uint8_t cc_num, uint8_t enable);
void hal_CC_Pin_Set(uint8_t cc_num, CC_PIN pin, uint8_t state);
uint32_t hal_CC_Pin_Get(uint8_t cc_num, CC_PIN pin);

#ifdef __cplusplus
}
#endif

#endif
