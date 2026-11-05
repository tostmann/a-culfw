/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Dirk Tostmann / busware
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/*
 * dudette_hal.c - dud_hal_* of dudette_stick.c on an AVR CUL.
 *
 * The radio is polled, like in rf_asksin.c: GDO2 (CC1100_IN) rises on a packet
 * with a good CRC. While Dudette owns the radio the main loop runs none of the
 * other RF tasks, see CUL.c.
 */
#include "board.h"

#ifdef HAS_DUDETTE

#include <Drivers/USB/HighLevel/USBTask.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "cc1100.h"
#include "cdc.h"
#include "clock.h"
#include "delay.h"
#include "display.h"
#include "dudette_stick.h"
#include "led.h"
#include "rf_asksin.h"
#include "rf_receive.h"
#include "rf_send.h"

#define DUD_MAX_RAHMEN  58
#define DUD_PA          0xC0    /* Dudette src/cc1101_regs.h CC_PA_VALUE */

/*
 * Dudette radio settings, section 1 of the protocol: 868.4998 MHz, 99.976 kBaud
 * GFSK, sync 0D3B, variable length with CRC and whitening.
 *
 * GDO0 stays high impedance (0x2E, as in every culfw mode): on the CUL it is
 * wired to PD3, which tx_init() drives as the OOK data output. The bootloader's
 * table sets 0x06 here, but the GDO assignment is local to the board.
 *
 * TEST2/1/0 are the reset values: FIFOTHR.ADC_RETENTION is off, and the AskSin
 * values only hold for filters below 325 kHz (SWRS061I p. 72).
 */
static const uint8_t PROGMEM DUD_CFG[] = {
  0x00, 0x07,  0x02, 0x2E,  0x03, 0x0D,  0x04, 0x0D,  0x05, 0x3B,
  0x06, 0x3A,  0x07, 0x0C,  0x08, 0x45,  0x09, 0x00,  0x0A, 0x00,
  0x0B, 0x0C,  0x0C, 0x00,
  0x0D, 0x21,  0x0E, 0x67,  0x0F, 0x62,
  0x10, 0x5B,  0x11, 0xF8,  0x12, 0x13,  0x13, 0x22,  0x14, 0xF8,
  0x15, 0x50,  0x16, 0x07,  0x17, 0x0F,  0x18, 0x18,
  0x19, 0x1D,  0x1A, 0x1C,  0x1B, 0xC7,  0x1C, 0x00,  0x1D, 0xB0,
  0x21, 0xB6,  0x22, 0x10,  0x23, 0xEA,  0x24, 0x2A,  0x25, 0x00,  0x26, 0x1F,
  0x2C, 0x88,  0x2D, 0x31,  0x2E, 0x0B,
};

/* Debug channel (Dudette include/dudette_debug.h): 869.525 MHz, sync D1DB. */
static const uint8_t PROGMEM DUD_CFG_DEBUG[] = {
  0x04, 0xD1,  0x05, 0xDB,  0x0D, 0x21,  0x0E, 0x71,  0x0F, 0x7A,
};

static void
tabelle(const uint8_t *t, uint8_t n)
{
  for (uint8_t i = 0; i < n; i += 2)
    cc1100_writeReg(pgm_read_byte(t + i), pgm_read_byte(t + i + 1));
}

static void
rx_an(void)
{
  uint8_t cnt = 0xff;
  do {
    ccStrobe(CC1100_SRX);
    my_delay_us(10);
  } while (cnt-- && cc1100_readReg(CC1100_MARCSTATE) != MARCSTATE_RX);
}

static void
flush_rx(void)
{
  ccStrobe(CC1100_SIDLE);
  ccStrobe(CC1100_SFRX);
  rx_an();
}

/* 1 ms from Timer0: ticks is 125 Hz, TCNT0 counts 250 steps of 32 us per tick. */
uint32_t
dud_hal_ms(void)
{
  uint32_t t;
  uint8_t c, sreg = SREG;

  cli();
  t = ticks;
  c = TCNT0;
  if (TIFR0 & _BV(OCF0A)) {     // the tick is pending, ticks is one behind
    t++;
    c = TCNT0;
  }
  SREG = sreg;
  return (t << 3) + (c >> 5);
}

void
dud_hal_funk(uint8_t modus)
{
  if (modus == DUD_FUNK_HMIP) { // back to whatever the user had running
    if (asksin_on || hmip_on)
      rf_asksin_init();
    else
      set_txrestore();
    return;
  }

  EIMSK &= ~_BV(CC1100_INT);    // no SlowRF edges, we poll

  CC1100_DEASSERT;              // same reset sequence as rf_asksin_init()
  my_delay_us(30);
  CC1100_ASSERT;
  my_delay_us(30);
  CC1100_DEASSERT;
  my_delay_us(45);
  ccStrobe(CC1100_SRES);
  my_delay_us(100);

  tabelle(DUD_CFG, sizeof(DUD_CFG));
  if (modus == DUD_FUNK_DEBUG)
    tabelle(DUD_CFG_DEBUG, sizeof(DUD_CFG_DEBUG));
  cc1100_writeReg(CC1100_PATABLE, DUD_PA);

  cc_freqcal();                 // the crystal offset of this stick, all channels
  ccStrobe(CC1100_SCAL);
  my_delay_ms(4);
  rx_an();
}

uint8_t
dud_hal_rx(uint8_t *buf, int8_t *rssi, uint8_t *lqi)
{
  uint8_t n, r, s;

  if (!bit_is_set(CC1100_IN_PORT, CC1100_IN_PIN)) {
    switch (cc1100_readReg(CC1100_MARCSTATE)) {
      case MARCSTATE_RXFIFO_OVERFLOW:
        flush_rx();
        break;
      case MARCSTATE_IDLE:
        rx_an();
        break;
    }
    return 0;
  }

  n = cc1100_readReg(CC1100_RXFIFO);
  if (n == 0 || n > DUD_MAX_RAHMEN) {
    flush_rx();
    return 0;
  }

  CC1100_ASSERT;
  cc1100_sendbyte(CC1100_READ_BURST | CC1100_RXFIFO);
  for (uint8_t i = 0; i < n; i++)
    buf[i] = cc1100_sendbyte(0);
  r = cc1100_sendbyte(0);
  s = cc1100_sendbyte(0);
  CC1100_DEASSERT;

  *rssi = (int8_t)((int8_t)r / 2 - 74);
  *lqi = s & 0x7F;
  return (s & 0x80) ? n : 0;    // CRC_OK
}

uint8_t
dud_hal_tx(const uint8_t *f, uint8_t n)
{
  uint16_t guard;
  uint8_t ok = 0;

  if (n == 0 || n > DUD_MAX_RAHMEN)
    return 0;

  ccStrobe(CC1100_SIDLE);
  ccStrobe(CC1100_SFRX);
  ccStrobe(CC1100_SFTX);

  CC1100_ASSERT;
  cc1100_sendbyte(CC1100_WRITE_BURST | CC1100_TXFIFO);
  cc1100_sendbyte(n);
  for (uint8_t i = 0; i < n; i++)
    cc1100_sendbyte(f[i]);
  CC1100_DEASSERT;

  LED_ON();
  ccStrobe(CC1100_STX);
  for (guard = 0; guard < 300; guard++) {
    if (cc1100_readReg(CC1100_MARCSTATE) == MARCSTATE_TX)
      break;
    my_delay_us(100);
    if ((guard % 100) == 99)
      ccStrobe(CC1100_STX);
  }

  // Do not poll during the frame. 100 kBaud: 80 us per byte, plus preamble 4,
  // sync 4, length 1, CRC 2. MCSM1 returns to RX by itself: the bootloader
  // answers within a few milliseconds (section 10.8).
  my_delay_us(((uint16_t)n + 11) * 80 + 500);
  for (guard = 0; guard < 50; guard++) {
    if ((cc1100_readReg(CC1100_TXBYTES) & 0x7F) == 0) {
      ok = n;
      break;
    }
    my_delay_us(500);
  }
  LED_OFF();

  if (!ok) {
    flush_rx();
    return 0;
  }
  if (cc1100_readReg(CC1100_MARCSTATE) != MARCSTATE_RX)
    rx_an();
  return ok;
}

/*
 * The same 1 % account as every other sender here. Booked is the real air time
 * at 100 kBaud, (n + 11) bytes of 80 us; rounding a frame up to 10 ms would make
 * an image cost twice what it does, so the remainder is carried in microseconds.
 */
uint8_t
dud_hal_konto(uint8_t n)
{
  static uint16_t rest_us;
  uint32_t us = rest_us + ((uint32_t)n + 11) * 80;
  uint16_t einheiten = (uint16_t)(us / 10000);

  if (credit_10ms < einheiten)
    return 0;
  credit_10ms -= einheiten;
  rest_us = (uint16_t)(us % 10000);
  return 1;
}

/*
 * Blocking deadlines reach 6 s: the watchdog is 2 s and USB control requests
 * must not starve. Input stays in the endpoint meanwhile (CDC_Task does not read
 * while a command runs), so the host is held off by NAK and nothing is lost.
 */
void
dud_hal_warten(void)
{
  Minute_Task();                // wdt_reset, 1 % credit
  USB_USBTask();
  CDC_Task();
}

void
dud_hal_putc(char c)
{
  DC(c);
}

void
dud_hal_zeilenende(void)
{
  DNL();
}

#endif
