#include <Arduino.h>
#include "board.h"
#include "hal_gpio.h"
#include "hal_spi.h"
#include "hal_timer.h"

extern "C" {
#include "delay.h"
#include "clock.h"
#include "rf_receive.h"
#include "rf_send.h"
#include "cc1100.h"
#include "ttydata.h"
#include "ringbuffer.h"
#include "fncollection.h"
#include "display.h"
#include "fastrf.h"
#include "fband.h"
#include "rf_mode.h"
#include "version.h"
#include "fht.h"

extern volatile uint32_t gdo_isr_count;
extern "C" uint8_t bucket_nrused[];

#ifdef HAS_ASKSIN
#include "rf_asksin.h"
#endif
#ifdef HAS_MORITZ
#include "rf_moritz.h"
#endif
#ifdef HAS_RWE
#include "rf_rwe.h"
#endif
#ifdef HAS_RFNATIVE
#include "rf_native.h"
#endif
#ifdef HAS_IT
#include "rf_receive_it.h"
#endif
#ifdef HAS_SOMFY_RTS
#include "somfy_rts.h"
#endif
#ifdef HAS_MBUS
#include "rf_mbus.h"
#endif

void eeprom_init(void);
extern const t_fntab fntab[];
void hal_enable_CC_GDOin_int(uint8_t cc_num, uint8_t enable);

void show_debug(char *in) {
    display_debug(in);
}
}

#define rb_is_empty(rb) ((rb)->nbytes == 0)

extern "C" const t_fntab fntab[] = {
  { 'C', ccreg },
  { 'F', fs20send },
#ifdef HAS_IT
  { 'i', it_func },
#endif
#ifdef HAS_ASKSIN
  { 'A', asksin_func },
#endif
#ifdef HAS_MORITZ
  { 'Z', moritz_func },
#endif
#ifdef HAS_RFNATIVE
  { 'N', native_func },
#endif
#ifdef HAS_RWE
  { 'E', rwe_func },
#endif
  { 'R', read_eeprom },
  { 'T', fhtsend },
  { 'V', version },
  { 'W', write_eeprom },
  { 'X', set_txreport },
  { 'e', eeprom_factory_reset },
#ifdef HAS_FASTRF
  { 'f', fastrf_func },
#endif
  { 'l', ledfunc },
  { 't', gettime },
  { 'x', ccsetpa },
  { 'y', show_debug },
#ifdef HAS_MBUS
  { 'b', rf_mbus_func },
#endif
#ifdef HAS_SOMFY_RTS
  { 'Y', somfy_rts_func },
#endif
  { 0, 0 },
};

void background_task(void *pvParameters) {
    for(;;) {
        hal_timer_task();
        RfAnalyze_Task();
        yield();
    }
}

void setup() {
    // USB CDC Initialisierung für ESP32-C3
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0); // Non-blocking
    
    // Warte kurz auf Serial, aber nicht ewig (verhindert hängen ohne USB)
    unsigned long start = millis();
    while (!Serial && (millis() - start) < 2000);

    Serial.println("\r\n--- CULFW32 Starting (XIAO Safe Pinning) ---");
    
    hal_timer_init();
    eeprom_init();
    
    rb_reset(&TTY_Rx_Buffer);
    rb_reset(&TTY_Tx_Buffer);
    
    input_handle_func = analyze_ttydata;
    display_channel = DISPLAY_USB;
    
    spi_init();
    ccInitChip(EE_CC1100_CFG);
    tx_init();
    
    // Background task für Timer und Analyse (höhere Prio als loop)
    xTaskCreate(background_task, "bg", 4096, NULL, 10, NULL);

    // Interrupt erst nach Initialisierung aktivieren
    hal_enable_CC_GDOin_int(0, 1);
    
    Serial.println("Ready.");
}

void loop() {
    static uint32_t last_isr_print = 0;
    if (millis() - last_isr_print > 1000) {
        last_isr_print = millis();
        Serial.printf("ISR Count: %u, Ticks: %u, BucketUsed: %u\n", gdo_isr_count, ticks, bucket_nrused[0]);
    }

    // 1. Read from Serial into Rx Buffer
    while (Serial.available()) {
        uint8_t c = Serial.read();
        rb_put(&TTY_Rx_Buffer, c);
    }
    
    // 2. Process Commands
    if (!rb_is_empty(&TTY_Rx_Buffer)) {
        // Serial.printf("Processing %d bytes\n", TTY_Rx_Buffer.nbytes);
        analyze_ttydata(DISPLAY_USB);
    }
    
    // 3. Background Tasks
    Minute_Task();
    
#ifdef HAS_FASTRF
    FastRF_Task();
#endif
#ifdef HAS_ASKSIN
    rf_asksin_task();
#endif
#ifdef HAS_MORITZ
    rf_moritz_task();
#endif
#ifdef HAS_RWE
    rf_rwe_task();
#endif
#ifdef HAS_MBUS
    rf_mbus_task();
#endif
#ifdef HAS_RFNATIVE
    native_task();
#endif

    // 4. Write from Tx Buffer to Serial
    while (!rb_is_empty(&TTY_Tx_Buffer)) {
        Serial.write(rb_get(&TTY_Tx_Buffer));
    }
    
    yield();
}
