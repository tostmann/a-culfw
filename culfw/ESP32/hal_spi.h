#ifndef __HAL_SPI_H
#define __HAL_SPI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void spi_init(void);
uint8_t spi_send(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif
