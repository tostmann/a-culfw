#ifndef _AVR_INTERRUPT_H
#define _AVR_INTERRUPT_H
#pragma GCC system_header

#ifdef __cplusplus
extern "C" {
#endif

// Direct FreeRTOS / ESP32-HAL calls to avoid circularity with Arduino.h
void vPortEnterCritical();
void vPortExitCritical();

#define cli() portDISABLE_INTERRUPTS()
#define sei() portENABLE_INTERRUPTS()

#ifdef __cplusplus
}
#endif

#endif
