#ifndef __HAL_H
#define __HAL_H

#include "hal_gpio.h"
#include "hal_spi.h"
#include "hal_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

void hal_eeprom_init(void);

#ifdef __cplusplus
}
#endif

#endif
