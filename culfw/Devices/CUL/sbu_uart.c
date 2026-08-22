#include "sbu_uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ttydata.h"
#include "display.h"
#include "ringbuffer.h"

#ifdef TTYSBU

// Assuming F_CPU is defined
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

// Detect Hardware Revision
// PB4 is LOW if SBU board (pulled down externally)
#define SBU_DETECT_DDR  DDRB
#define SBU_DETECT_PIN  PINB
#define SBU_DETECT_BIT  4
#define SBU_MUX_DDR     DDRD
#define SBU_MUX_PORT    PORTD
#define SBU_MUX_BIT     4

uint8_t sbu_mode = 0;

// rb_t includes the buffer array internally based on TTY_BUFSIZE from board.h
static rb_t SBU_Rx_Buffer;
static rb_t SBU_Tx_Buffer;

// We use TTY_BUFSIZE for the command buffer as well to be consistent
static char sbu_cmdbuf[TTY_BUFSIZE+1];
static uint8_t sbu_cmdlen = 0;

void sbu_init(void) {
    // 1. Check for SBU
    // Configure PB4 as input with pullup
    SBU_DETECT_DDR &= ~_BV(SBU_DETECT_BIT); // Input
    PORTB |= _BV(SBU_DETECT_BIT);
    _delay_ms(1);
    
    // Check if LOW
    if (!(SBU_DETECT_PIN & _BV(SBU_DETECT_BIT))) {
        sbu_mode = 1;
        // 2. Switch Multiplexer
        SBU_MUX_DDR |= _BV(SBU_MUX_BIT);    // Output
        SBU_MUX_PORT |= _BV(SBU_MUX_BIT);   // HIGH -> SBU Mode
        
        // 3. Init UART1 (38400)
        uint32_t baud = 38400;
        uint16_t ubrr = (F_CPU / 16 / baud) - 1;
        
        UBRR1H = (uint8_t)(ubrr >> 8);
        UBRR1L = (uint8_t)ubrr;
        
        // Enable receiver and transmitter, Enable RX Interrupt
        // We do NOT enable TX Interrupt yet, only when we have data
        UCSR1B = (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1);
        
        // Set frame format: 8data, 1stop bit (default 8N1)
        UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);

    } else {
        sbu_mode = 0;
        SBU_MUX_DDR |= _BV(SBU_MUX_BIT);
        SBU_MUX_PORT &= ~_BV(SBU_MUX_BIT); // LOW -> Legacy Mode
    }
}

// RX Interrupt
ISR(USART1_RX_vect) {
    uint8_t c = UDR1;
    rb_put(&SBU_Rx_Buffer, c);
}

// TX Interrupt
ISR(USART1_UDRE_vect) {
    if (SBU_Tx_Buffer.nbytes) {
        UDR1 = rb_get(&SBU_Tx_Buffer);
    } else {
        // Disable UDRE interrupt if nothing to send
        UCSR1B &= ~(1<<UDRIE1);
    }
}

void sbu_task(void) {
    if (!sbu_mode) return;
    
    // Process Received Data
    uint8_t odc = display_channel;
    display_channel = DISPLAY_SBU;
    
    while(SBU_Rx_Buffer.nbytes) {
        cli();
        if (SBU_Rx_Buffer.nbytes == 0) {
            sei();
            break;
        }
        uint8_t c = rb_get(&SBU_Rx_Buffer);
        sei();
        
        if(c == '\n' || c == '\r') {
            if(!sbu_cmdlen) continue;
            
            sbu_cmdbuf[sbu_cmdlen] = 0;
            
            // Execute command
            if(!callfn(sbu_cmdbuf)) {
                 // DS_P(PSTR("?")); // Optional: Error feedback
            }
            sbu_cmdlen = 0;
        } else {
            if(sbu_cmdlen < TTY_BUFSIZE)
                sbu_cmdbuf[sbu_cmdlen++] = c;
        }
    }
    display_channel = odc;
}

void sbu_putc(char c) {
    if (!sbu_mode) return;
    
    // If buffer full, wait (busy wait, unfortunate but necessary if overflow)
    // In ISR context this could be bad, but usually output is faster than generation or buffer is large enough
    while (*(volatile uint8_t *)&SBU_Tx_Buffer.nbytes >= TTY_BUFSIZE) {
        // Process Interrupts while waiting? 
        // AVR Interrupts are enabled globally, so UDRE ISR will drain buffer eventually.
        // We just spin here.
    }
    
    // Disable interrupts atomically to modify buffer? 
    // rb_put handles pointers. But we need to be careful with UDRE ISR potentially modifying nbytes/getoff
    // rb_put implementation in ringbuffer.c is not atomic!
    // We should mask interrupts around rb_put if ISR also accesses it?
    // ISR accesses: rb_get (consumes). sbu_putc calls rb_put (produces).
    // Single producer, single consumer.
    // rb_put modifies: putoff, nbytes.
    // rb_get modifies: getoff, nbytes.
    // Conflict on 'nbytes'.
    
    cli();
    rb_put(&SBU_Tx_Buffer, (uint8_t)c);
    
    // Enable UDRE Interrupt to start transmission
    UCSR1B |= (1<<UDRIE1);
    sei();
}

#else
/* Without TTYSBU this file is empty on purpose. sbu_uart.h already turns
   sbu_mode, sbu_init(), sbu_task() and sbu_putc() into no-op macros, so
   defining functions of the same name here would be macro-expanded and
   fail to compile. */
#endif
