#include <Arduino.h>
#include "board.h"
#include "hal_gpio.h"
#include "hal_spi.h"
#include "hal_timer.h"

static uint32_t last_scan[32];

extern "C" {
volatile uint32_t toggle_count[32];

#include "delay.h"
#include "clock.h"
#include "rf_receive.h"
#include "rf_send.h"
#include "cc1100.h"
#include "ttydata.h"
#include "ringbuffer.h"
#include "fncollection.h"
extern "C" {
#include "display.h"
#include "fastrf.h"
#include "fband.h"
#include "rf_mode.h"
#include "version.h"
#include "fht.h"
}

extern volatile uint32_t gdo_isr_count;
extern volatile uint32_t gdo_high_count;
extern volatile uint32_t gdo_low_count;
extern "C" uint8_t bucket_nrused[];

extern "C" {
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
}

void eeprom_init(void);
extern const t_fntab fntab[];
void hal_enable_CC_GDOin_int(uint8_t cc_num, uint8_t enable);

extern "C" {
#include "rf_receive_bucket.h"
extern bucket_t bucket_array[NUM_SLOWRF][RCV_BUCKETS];
extern volatile uint8_t bucket_in[NUM_SLOWRF];
extern volatile pulse_t hightime[NUM_SLOWRF], lowtime[NUM_SLOWRF];
}

void show_debug(char *in) {
    display_debug(in);
    DS_P(PSTR(" ISR:")); DU(gdo_isr_count, 5);
    DS_P(PSTR(" H/L:")); DU(hightime[0], 5); DC('/'); DU(lowtime[0], 5);
    
    bucket_t *b = &bucket_array[0][bucket_in[0]];
    DS_P(PSTR(" ST:")); DU(b->state, 2);
    DS_P(PSTR(" SY:")); DU(b->sync, 2);
    DS_P(PSTR(" Z:")); DU(b->zero.hightime, 4); DC('/'); DU(b->zero.lowtime, 4);
    DS_P(PSTR(" O:")); DU(b->one.hightime, 4); DC('/'); DU(b->one.lowtime, 4);
    
    DS_P(PSTR(" F:")); DU(frequencyMode, 1);
    DS_P(PSTR(" I433:")); DU(IS433MHZ, 1);
    DS_P(PSTR(" B:")); DU(bucket_nrused[0], 2);
    DNL();
}

static void send_man_bit(uint16_t us) {
    hal_CC_Pin_Set(0, CC_Pin_Out, 1);
    delayMicroseconds(us);
    hal_CC_Pin_Set(0, CC_Pin_Out, 0);
    delayMicroseconds(us);
}

void test_pulse(char *in) {
    display_debug(in);
    DS_P(PSTR(" Pst:"));
    // Original diagnostic function
    if (in[1] == '0') digitalWrite(LED_PIN, LOW);
    if (in[1] == '1') digitalWrite(LED_PIN, HIGH);
    DS_P(PSTR(" Done"));
    DNL();
}
}

#define rb_is_empty(rb) ((rb)->nbytes == 0)

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
  { 'y', show_debug },
  { 'P', test_pulse },
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
        vTaskDelay(10); 
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0); 
    delay(2000);
    Serial.println("\n--- CULFW32 Starting ---");

    hal_timer_init();
    eeprom_init();
    
    rb_reset(&TTY_Rx_Buffer);
    rb_reset(&TTY_Tx_Buffer);
    
    input_handle_func = analyze_ttydata;
    display_channel = DISPLAY_USB;
    
    spi_init();
    ccInitChip(EE_CC1100_CFG);
    checkFrequency();
    ccRX();
    tx_init();
    
    xTaskCreate(background_task, "bg", 8192, NULL, 1, NULL);
    hal_enable_CC_GDOin_int(0, 1);
    
    Serial.println("Full Init done.");
}

void loop() {
    while (Serial.available()) {
        uint8_t c = Serial.read();
        rb_put(&TTY_Rx_Buffer, c);
    }
    
    if (!rb_is_empty(&TTY_Rx_Buffer)) {
        analyze_ttydata(DISPLAY_USB);
    }
    
    Minute_Task();
    
    if (!rb_is_empty(&TTY_Tx_Buffer)) {
        size_t available = Serial.availableForWrite();
        while (available > 0 && !rb_is_empty(&TTY_Tx_Buffer)) {
            Serial.write(rb_get(&TTY_Tx_Buffer));
            available--;
        }
    }
    
    vTaskDelay(1);
}
