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
 * dudette_stick.h - stick side of the Dudette radio bootloader.
 *
 * Written against Dudette doc/protocol.md section 10 (stick grammar) and
 * sections 1 to 3 alone. The module is target-neutral: everything that depends
 * on chip and board goes through the dud_hal_* functions below.
 */
#ifndef _DUDETTE_STICK_H
#define _DUDETTE_STICK_H

#include <stdint.h>

/* Wem das Funkteil gerade gehoert. */
#define DUD_FUNK_HMIP    0   /* Dudette ruht, das Funkteil gehoert dem Rest */
#define DUD_FUNK_BOOT    1   /* 868,4998 MHz, Sync 0D3B                    */
#define DUD_FUNK_DEBUG   2   /* 869,5250 MHz, Sync D1DB                    */

/* ---- vom Modul angeboten ------------------------------------------------ */

void    dud_init(void);
/* Eine Befehlszeile, die mit 'D' beginnt. Antwortet selbst. */
void    dud_befehl(const char *zeile);
/* Aus der Hauptschleife, solange dud_funk() != DUD_FUNK_HMIP. */
void    dud_task(void);
/* Wem das Funkteil gehoert — die Hauptschleife fragt das vor radio_poll(),
 * der Sendeweg vor jeder HmIP/BidCoS-Sendung. */
uint8_t dud_funk(void);

/* ---- vom Ziel bereitzustellen ------------------------------------------- */

uint32_t dud_hal_ms(void);
/* Funkteil umstellen: DUD_FUNK_HMIP stellt den HmIP-Registersatz wieder her. */
void     dud_hal_funk(uint8_t modus);
/* Rahmen abholen (ohne Laengenbyte, Pruefsumme schon geprueft).
 * Rueckgabe Laenge oder 0; rssi in dBm, lqi 0..127. */
uint8_t  dud_hal_rx(uint8_t *buf, int8_t *rssi, uint8_t *lqi);
/* Rahmen senden; 1 = hinaus, 0 = gescheitert. */
uint8_t  dud_hal_tx(const uint8_t *f, uint8_t n);
/* Sendezeit fuer einen Rahmen von n Byte buchen; 1 = gebucht, 0 = Konto leer. */
uint8_t  dud_hal_konto(uint8_t n);
/* Waehrend einer blockierenden Frist: Schnittstelle bedienen. */
void     dud_hal_warten(void);
/* Output: one character, and the end of a line (CR LF). */
void     dud_hal_putc(char c);
void     dud_hal_zeilenende(void);

#endif
