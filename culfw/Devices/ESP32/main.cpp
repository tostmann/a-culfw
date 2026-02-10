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

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\r\n!!! SIMPLE BOOT TEST !!!");
}

void loop() {
    Serial.println("Alive...");
    delay(1000);
}
