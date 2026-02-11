#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include "hal_gpio.h"
#include "hal_spi.h"
#include "hal_timer.h"
#include "board.h"
#include "esp_timer.h"

extern "C" {
  volatile uint32_t ticks = 0;
  uint8_t SREG = 0;
  uint8_t TIMSK0 = 0;
}
volatile uint32_t gdo_isr_count = 0;
volatile uint32_t gdo_high_count = 0;
volatile uint32_t gdo_low_count = 0;
volatile uint32_t last_isr_time_val = 0;

extern "C" {
#include "delay.h"
#include "rf_receive.h"

// Forward declarations of CULFW functions
void rf_receive_TimerElapsedCallback(void);
void CC1100_in_callback(void);
void clock_TimerElapsedCallback(void);
}

void IRAM_ATTR gdo_interrupt_handler();

// Timer for RF Silence timeout
static hw_timer_t * rf_hw_timer = NULL;
static volatile uint32_t rf_reload_val = 4000;
static volatile bool rf_timer_enabled = false;
static int gdo0_current_mode = -1;

void IRAM_ATTR onRfTimer() {
    rf_receive_TimerElapsedCallback();
}

void hal_timer_init(void) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    rf_hw_timer = timerBegin(1000000); // 1MHz
    timerAttachInterrupt(rf_hw_timer, &onRfTimer);
    timerStart(rf_hw_timer);
#else
    rf_hw_timer = timerBegin(1, 80, true); // 1MHz
    timerAttachInterrupt(rf_hw_timer, &onRfTimer, true);
#endif
}

uint32_t hal_get_ticks(void) {
    return ticks;
}

void hal_timer_task(void) {
    static uint32_t last_tick_ms = 0;
    uint32_t now = millis();
    if (now - last_tick_ms >= 8) { // 125Hz approx
        last_tick_ms = now;
        clock_TimerElapsedCallback();
    }
}

// RF Timer HAL
void IRAM_ATTR hal_enable_CC_timer_int(uint8_t instance, uint8_t enable) {
    if (enable) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
        timerAlarm(rf_hw_timer, rf_reload_val, false, 0);
        timerRestart(rf_hw_timer);
        timerStart(rf_hw_timer);
#else
        timerAlarmWrite(rf_hw_timer, rf_reload_val, false);
        timerAlarmEnable(rf_hw_timer);
        timerWrite(rf_hw_timer, 0);
#endif
        rf_timer_enabled = true;
    } else {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
        timerStop(rf_hw_timer);
#else
        timerAlarmDisable(rf_hw_timer);
#endif
        rf_timer_enabled = false;
    }
}

uint32_t HAL_timer_get_reload_register(uint8_t instance) {
    return rf_reload_val;
}

void HAL_timer_set_reload_register(uint8_t instance, uint32_t value) {
    rf_reload_val = value;
    if (rf_timer_enabled) {
        hal_enable_CC_timer_int(instance, true);
    }
}

static uint32_t rf_counter_offset = 0;
uint32_t IRAM_ATTR HAL_timer_get_counter_value(uint8_t instance) {
    return (uint32_t)esp_timer_get_time() - rf_counter_offset;
}

void HAL_timer_set_counter_value(uint8_t instance, uint32_t value) {
    rf_counter_offset = (uint32_t)esp_timer_get_time() - value;
}

void IRAM_ATTR HAL_timer_reset_counter_value(uint8_t instance) {
    rf_counter_offset = (uint32_t)esp_timer_get_time();
}

// GPIO stuff
void HAL_LED_Init(void) {
    pinMode(LED_PIN, OUTPUT);
}

void HAL_LED_Set(LED_List led, LED_State state) {
    digitalWrite(LED_PIN, state == LED_on ? HIGH : LOW);
}

void HAL_LED_Toggle(LED_List led) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void IRAM_ATTR gdo_interrupt_handler() {
    last_isr_time_val = (uint32_t)esp_timer_get_time();
    gdo_isr_count++;
    if (digitalRead(GDO0_PIN)) gdo_high_count++;
    else gdo_low_count++;
    if (rf_timer_enabled) {
        timerWrite(rf_hw_timer, 0);
    }
    CC1100_in_callback();
}

void hal_CC_GDO_init(uint8_t cc_num, uint8_t mode) {
    pinMode(CC1100_CS_PIN, OUTPUT);
    digitalWrite(CC1100_CS_PIN, HIGH);
    
    pinMode(GDO0_PIN, INPUT);
    gdo0_current_mode = INPUT;
    pinMode(GDO2_PIN, INPUT);
}

void hal_enable_CC_GDOin_int(uint8_t cc_num, uint8_t enable) {
    if (enable) {
        if (gdo0_current_mode != INPUT) {
            pinMode(GDO0_PIN, INPUT);
            gdo0_current_mode = INPUT;
        }
        attachInterrupt(digitalPinToInterrupt(GDO0_PIN), gdo_interrupt_handler, CHANGE);
    } else {
        detachInterrupt(digitalPinToInterrupt(GDO0_PIN));
    }
}

#include "driver/gpio.h"

void hal_CC_Pin_Set(uint8_t cc_num, CC_PIN pin, uint8_t state) {
    if (pin == CC_Pin_CS) {
        digitalWrite(CC1100_CS_PIN, state);
    } else if (pin == CC_Pin_Out) {
        // Mode should have been set by ccTX
        digitalWrite(GDO0_PIN, state);
    }
}

uint32_t hal_CC_Pin_Get(uint8_t cc_num, CC_PIN pin) {
    if (pin == CC_Pin_In) {
        return digitalRead(GDO0_PIN);
    }
    return 0;
}

// SPI stuff
void spi_init(void) {
    // Antenna Switch for XIAO ESP32-C6: GPIO 14
    // Low = On-board Antenna, High = UFL
    pinMode(14, OUTPUT);
    digitalWrite(14, LOW); 
    Serial.println("Antenna set to PCB (GPIO 14 LOW)");

    pinMode(SPI_SCLK, OUTPUT);
    pinMode(SPI_MOSI, OUTPUT);
    pinMode(SPI_MISO, INPUT);
    pinMode(SPI_SS, OUTPUT);
    digitalWrite(SPI_SS, HIGH);
    
    // Pass -1 for SS to prevent SPI peripheral from taking over the pin
    SPI.begin(SPI_SCLK, SPI_MISO, SPI_MOSI, -1);
    SPI.setFrequency(2000000); // 2MHz for stability

#ifdef MARK433_PIN
    pinMode(MARK433_PIN, INPUT_PULLUP);
#endif
}

uint8_t spi_send(uint8_t data) {
    return SPI.transfer(data);
}

// Delay
extern "C" void my_delay_ms(uint8_t ms) {
    delay(ms);
}

extern "C" void my_delay_us(uint16_t us) {
    delayMicroseconds(us);
}

// EEPROM emulation
extern "C" void hal_eeprom_init(void) {
    if (!EEPROM.begin(512)) {
    }
}

extern "C" uint8_t eeprom_read_byte(uint8_t *addr) {
    return EEPROM.read((size_t)addr);
}

extern "C" void eeprom_write_byte(uint8_t *addr, uint8_t value) {
    EEPROM.write((size_t)addr, value);
    EEPROM.commit();
}
