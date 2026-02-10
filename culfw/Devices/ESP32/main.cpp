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
#ifdef HAS_INTERTECHNO
#include "intertechno.h"
#endif
#ifdef HAS_SOMFY_RTS
#include "somfy_rts.h"
#endif
#ifdef HAS_MBUS
#include "rf_mbus.h"
#endif

void eeprom_init(void);

// Function table
extern const t_fntab fntab[];

void hal_enable_CC_GDOin_int(uint8_t cc_num, uint8_t enable);
}

#define rb_is_empty(rb) ((rb)->nbytes == 0)

// Function table definition
extern "C" const t_fntab fntab[] = {
  { 'C', ccreg },
  { 'F', fs20send },
#ifdef HAS_INTERTECHNO
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
#ifdef HAS_MBUS
  { 'b', rf_mbus_func },
#endif
#ifdef HAS_SOMFY_RTS
  { 'Y', somfy_rts_func },
#endif
  { 0, 0 },
};

#if !ARDUINO_USB_CDC_ON_BOOT
#define Serial USBSerial
#endif

void setup() {
    Serial.begin(115200);
    
    long start = millis();
    while (!Serial && millis() - start < 5000); 

    Serial.println("\r\n!!! CULFW32 BOOT START !!!");
    Serial.println("Serial started.");
    
    Serial.println("Timer Init...");
    hal_timer_init();
    Serial.println("EEPROM Init...");
    eeprom_init();
    
    rb_reset(&TTY_Rx_Buffer);
    rb_reset(&TTY_Tx_Buffer);
    
    input_handle_func = analyze_ttydata;
    display_channel = DISPLAY_USB;
    
    Serial.println("SPI Init...");
    spi_init();
    Serial.println("CC1101 Init...");
    ccInitChip(EE_CC1100_CFG);
    Serial.println("TX Init...");
    tx_init();
    
    hal_enable_CC_GDOin_int(0, 1);
    
    Serial.println("Ready.");
}

void loop() {
    static uint32_t last_isr_count = 0;
    static unsigned long last_debug_print = 0;

    if (millis() - last_debug_print > 1000) {
        Serial.print("Heartbeat. ISR Count: ");
        Serial.println(gdo_isr_count);
        last_isr_count = gdo_isr_count;
        last_debug_print = millis();
    }

    // 1. Read from Serial into Rx Buffer
    while (Serial.available()) {
        uint8_t c = Serial.read();
        rb_put(&TTY_Rx_Buffer, c);
    }
    
    // 2. Process Commands
    if (!rb_is_empty(&TTY_Rx_Buffer)) {
        analyze_ttydata(DISPLAY_USB);
    }
    
    // 3. Background Tasks
    Minute_Task();
    RfAnalyze_Task();
    
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
        if (Serial.availableForWrite() > 0) {
            Serial.write(rb_get(&TTY_Tx_Buffer));
        } else {
            // Check if we are connected, otherwise discard to prevent hang
            // On ESP32 USB CDC, availableForWrite() might return 0 if no host is listening
            break; 
        }
    }
    
    // Allow ESP32 system tasks (Watchdog, etc)
    yield();
}
