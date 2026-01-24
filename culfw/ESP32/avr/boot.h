#ifndef _AVR_BOOT_H_
#define _AVR_BOOT_H_
#include <Arduino.h>
#define BOOTLOADER_SECTION
static uint8_t boot_special_request = 0;
static uint8_t boot_key = 0;
#endif
