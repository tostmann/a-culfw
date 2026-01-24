#ifndef _AVR_WDT_H_
#define _AVR_WDT_H_

#include <Arduino.h>
#include <esp_task_wdt.h>

// AVR Konstanten
#define WDTO_15MS   15
#define WDTO_30MS   30
#define WDTO_60MS   60
#define WDTO_120MS  120
#define WDTO_250MS  250
#define WDTO_500MS  500
#define WDTO_1S     1000
#define WDTO_2S     2000
#define WDTO_4S     4000
#define WDTO_8S     8000

// Reset ist einfach
#define wdt_reset() esp_task_wdt_reset()

// Init für ESP32 Core v3.0+
static inline void wdt_enable(uint32_t timeout_ms) {
    // Falls timeout < 1ms, min. 1s nutzen (ESP WDT mag kleine Werte nicht)
    if (timeout_ms < 1000) timeout_ms = 1000;

    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = timeout_ms,
        .idle_core_mask = (1 << 0), // Core 0 überwachen
        .trigger_panic = true
    };
    esp_task_wdt_init(&twdt_config);
    esp_task_wdt_add(NULL);
}

#define wdt_disable() esp_task_wdt_delete(NULL)

#endif
