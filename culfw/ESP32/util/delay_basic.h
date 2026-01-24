#ifndef _UTIL_DELAY_BASIC_H_
#define _UTIL_DELAY_BASIC_H_

#include <stdint.h>
#include <Arduino.h> // Für F_CPU und Arch-Defines

static inline uint32_t get_cpu_cycle_count(void) {
    uint32_t ccount;
    #if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(__riscv)
        // RISC-V Architektur (ESP32-C3)
        asm volatile("csrr %0, mcycle" : "=r"(ccount));
    #else
        // Xtensa Architektur (ESP32, S2, S3)
        asm volatile("rsr %0,ccount":"=a" (ccount));
    #endif
    return ccount;
}

static inline void _delay_loop_2(uint16_t count) {
    if (count == 0) return;
    
    // Annahme: count basiert auf AVR 16MHz (4 Cycles/Count -> 0.25us)
    // Ziel: Gleiche Zeitdauer auf ESP32 warten.
    // ESP32 Cycles = count * 4 * (F_CPU_ESP / F_CPU_AVR)
    // Vereinfacht: count * (F_CPU / 4000000)
    
    uint32_t f_cpu = F_CPU; // z.B. 160000000
    uint32_t cycles_per_count = f_cpu / 4000000; // bei 160MHz -> 40 Cycles
    
    uint32_t wait_cycles = count * cycles_per_count;
    uint32_t start = get_cpu_cycle_count();
    
    while ((get_cpu_cycle_count() - start) < wait_cycles) {
        asm volatile ("nop");
    }
}

static inline void _delay_loop_1(uint8_t count) {
    // Ähnliche Logik, loop_1 ist 3 Cycles auf AVR
    uint32_t f_cpu = F_CPU;
    uint32_t cycles_per_count = (f_cpu / 16000000) * 3; 
    
    uint32_t wait_cycles = count * cycles_per_count;
    uint32_t start = get_cpu_cycle_count();
    while ((get_cpu_cycle_count() - start) < wait_cycles);
}

#endif
