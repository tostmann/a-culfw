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
 * dudette_stick.c - stick side of the Dudette radio bootloader.
 *
 * Implements the D commands of Dudette doc/protocol.md section 10 (contract,
 * 2026-09-15) and sections 1 to 3; the section numbers below refer to that
 * document. Nothing is taken from CULFW32.
 *
 * One thread, like the rest of culfw. Commands with a reply deadline (Ds, De,
 * Dm, Dv, DV, Dg) block and serve the host through dud_hal_warten(). The armed
 * SELECT (Dw) and the debug channel run in dud_task(): the client polls the
 * state there while the device is not in its window yet.
 *
 * No stdio: on AVR, format strings live in flash and go straight to the
 * console through o_f(), which knows only the conversions used here.
 */
#include "dudette_stick.h"

#include <stdarg.h>
#include <string.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#define FS(x)    PSTR(x)
#define RD(p)    pgm_read_byte(p)
#define IM_FLASH PROGMEM
#else
#define FS(x)    (x)
#define RD(p)    (*(p))
#define IM_FLASH
#endif

/* ---- Protokoll (§2, §3) ------------------------------------------------- */
#define MAGIC         0xD1
#define T_HELLO       0x01
#define T_SELECT      0x02
#define T_ERASE       0x04
#define T_DATA        0x05
#define T_COMMIT      0x06
#define T_COMMIT_ACK  0x07
#define T_VERIFY      0x08
#define T_VERIFY_ACK  0x09
#define T_GO          0x0A
#define T_STATUS      0x0C
#define T_PRINT       0x0D
#define T_CMD         0x0E
#define T_EE_VERIFY   0x11

#define KOPF          6
#define FRAG          49          /* Nutzdaten je DATA-Rahmen            */
#define MAX_RAHMEN    58          /* PKTLEN                               */
#define HELLO_BASIS   31
#define DIAG_LEN      22
/* Largest SPM page of an AVR. The fragment bitmap would allow 8 * FRAG = 392. */
#ifndef SEITE_MAX
#define SEITE_MAX     256
#endif
#define PAGE_EE       0x8000u

#define ST_BOARD      176         /* §10.5, Stick-eigene Codes            */
#define ST_KOLLISION  177
#define ST_KEINER     255

/* section 10.4 allows at most 16 entries; each costs 62 bytes of RAM. */
#ifndef LISTE_MAX
#define LISTE_MAX     8
#endif

/* Fristen (§10.6) */
#define SELECT_FRIST_MS   150
#define SELECT_ANLAEUFE   40
#define ERASE_FRIST_MS    6000
#define COMMIT_FRIST_MS   500
#define COMMIT_EE_MS      1500
#define VERIFY_FRIST_MS   4000
#define GO_FRIST_MS       500
#define DATA_ABSTAND_MS   4       /* das Geraet muss den FIFO leeren      */

/*
 * Rueckfall auf HmIP. Kein Klient schickt am Ende `Dx` (dudette_flash.py endet
 * nach `Dg`, der Browser beim Schliessen der Seite) — ohne Rueckfall bliebe die
 * HmIP-Zentrale nach jedem Flashvorgang stumm, bis jemand `Dx` tippt. Also:
 * ohne D-Befehl zurueck auf HmIP, ausser ein SELECT ist scharfgestellt (das
 * Werkzeug wartet bis zu 180 s auf das Geraet) oder der Debug-Kanal ist an
 * (Mitlesen ohne Befehle, dafuer die laengere Frist). Das ist eine Zutat dieses
 * Moduls, nicht Teil des Vertrags; der Zustand danach ist „Empfang aus"
 * (§10.3), Liste und Sitzungsziel bleiben.
 */
#ifndef RUECKFALL_MS                /* die Host-Pruefung setzt kuerzere Fristen */
#define RUECKFALL_MS       60000u
#endif
#ifndef RUECKFALL_DBG_MS
#define RUECKFALL_DBG_MS   900000u
#endif

/* Fehlernamen (§10.5): ein Wort ohne Leerzeichen, das ERR enthaelt. */
static const char E_TIMEOUT[] IM_FLASH = "ESP_ERR_TIMEOUT";
static const char E_STATE[]   IM_FLASH = "ESP_ERR_INVALID_STATE";
static const char E_ARG[]     IM_FLASH = "ESP_ERR_INVALID_ARG";
static const char E_LOVF[]    IM_FLASH = "ERR_LOVF";      /* Sendezeit-Konto leer */
static const char E_TX[]      IM_FLASH = "ERR_TX";        /* Funkteil hat nicht gesendet */

/* ---- Zustand (§10.3) ---------------------------------------------------- */
typedef struct {
  uint8_t  belegt;
  uint32_t addr;
  uint16_t board;
  uint8_t  sig[3];
  uint8_t  ver, caps, id;
  uint8_t  lot[6], die[4];
  uint16_t pg;
  uint32_t app;
  uint8_t  flags;
  int8_t   rssi;
  uint8_t  lqi;
  uint32_t t;
  uint16_t n;
  uint8_t  koll;
  uint8_t  hat_diag;
  uint8_t  diag[DIAG_LEN];
} eintrag_t;

static eintrag_t liste[LISTE_MAX];

enum { W_IDLE, W_ARMED, W_SENT, W_OK, W_FAIL };
static const char *w_name(uint8_t z)
{
  switch (z) {
  case W_ARMED: return FS("armed");
  case W_SENT:  return FS("sent");
  case W_OK:    return FS("OK");
  case W_FAIL:  return FS("FAIL");
  default:      return FS("idle");
  }
}

static uint8_t  funk;               /* DUD_FUNK_*                           */
static uint8_t  empfang;            /* Empfang an (Dr, Dw, Db)              */
static uint8_t  dbg_an;
static uint32_t dbg_filter;

static uint8_t  w_zustand = W_IDLE;
static uint32_t w_addr;
static uint16_t w_board;
static uint8_t  w_st = ST_KEINER;
static uint8_t  w_seq;
static uint32_t w_t;
static uint16_t w_fail_board;       /* b= fuer den BOARD-Anhang             */
static uint8_t  w_fail_koll;

static uint32_t ziel;               /* Sitzungsziel                         */
static uint16_t ziel_pg = 256;
static uint16_t ziel_board;

static uint16_t puf_nr = 0xFFFF;
static uint8_t  puf_map;
static uint8_t  puf[SEITE_MAX];

static uint8_t  seq;
static uint32_t letzter_befehl;     /* fuer den Rueckfall auf HmIP          */

/* Antwort, auf die eine blockierende Anfrage wartet */
static uint8_t  warte_typ, warte_seq;
static uint32_t warte_addr;
static uint8_t  antwort[MAX_RAHMEN + 4];
static uint8_t  antwort_n, antwort_da;

/* ---- Hilfen ------------------------------------------------------------- */

/*
 * Reads one byte of a format string. Not inline on purpose: with two RD(f++) in
 * a row, avr-gcc 5.4 and 7.3 took r31 as a scratch register while Z was still
 * live, and the second read came from the wrong address (ldi r31,4 before
 * adiw r30,1 in o_fv, seen on a CUL V3: "%06lX" printed "000000X").
 */
static char __attribute__((noinline)) rd(const char *p)
{
  return (char)RD(p);
}

static void o_zahl(unsigned long v, uint8_t basis, uint8_t breite)
{
  char b[10];
  uint8_t i = 0;
  do {
    uint8_t d = (uint8_t)(v % basis);
    b[i++] = (char)(d < 10 ? '0' + d : 'A' + d - 10);
    v /= basis;
  } while (v);
  while (breite > i) { dud_hal_putc('0'); breite--; }
  while (i) dud_hal_putc(b[--i]);
}

/* %s flash string, %r RAM string, %c, %d, %u, %lu, %0<n>X, %0<n>lX */
static void o_fv(const char *f, va_list ap)
{
  char c;
  while ((c = rd(f++))) {
    uint8_t breite = 0, lang = 0;
    if (c != '%') { dud_hal_putc(c); continue; }
    c = rd(f++);
    if (c == '0') { breite = (uint8_t)(rd(f++) - '0'); c = rd(f++); }
    if (c == 'l') { lang = 1; c = rd(f++); }
    switch (c) {
    case 's': {
      const char *p = va_arg(ap, const char *);
      char x;
      while ((x = rd(p++))) dud_hal_putc(x);
      break;
    }
    case 'r': {
      const char *p = va_arg(ap, const char *);
      while (*p) dud_hal_putc(*p++);
      break;
    }
    case 'c':
      dud_hal_putc((char)va_arg(ap, int));
      break;
    case 'd': {
      int v = va_arg(ap, int);
      if (v < 0) { dud_hal_putc('-'); v = -v; }
      o_zahl((unsigned long)v, 10, 0);
      break;
    }
    default: {                                  /* u, X */
      unsigned long v = lang ? va_arg(ap, unsigned long) : va_arg(ap, unsigned);
      o_zahl(v, c == 'X' ? 16 : 10, breite);
      break;
    }
    }
  }
}

static void o_f(const char *f, ...)
{
  va_list ap;
  va_start(ap, f);
  o_fv(f, ap);
  va_end(ap);
}

/* a whole line */
static void o_z(const char *f, ...)
{
  va_list ap;
  va_start(ap, f);
  o_fv(f, ap);
  va_end(ap);
  dud_hal_zeilenende();
}

static uint16_t crc16(const uint8_t *d, uint16_t n)
{
  uint16_t c = 0xFFFF;
  while (n--) {
    uint8_t i;
    c ^= (uint16_t)(*d++) << 8;
    for (i = 0; i < 8; i++)
      c = (c & 0x8000) ? (uint16_t)((c << 1) ^ 0x1021) : (uint16_t)(c << 1);
  }
  return c;
}

static int8_t hexziffer(char c)
{
  if (c >= '0' && c <= '9') return (int8_t)(c - '0');
  if (c >= 'a' && c <= 'f') return (int8_t)(c - 'a' + 10);
  if (c >= 'A' && c <= 'F') return (int8_t)(c - 'A' + 10);
  return -1;
}

/* n Zeichen hex nach *out; 1 = gueltig. n = 0 ergibt 0. */
static uint8_t hexzahl(const char *s, uint8_t n, uint32_t *out)
{
  uint32_t v = 0;
  uint8_t i;
  if (n > 8) return 0;
  for (i = 0; i < n; i++) {
    int8_t d = hexziffer(s[i]);
    if (d < 0) return 0;
    v = (v << 4) | (uint8_t)d;
  }
  *out = v;
  return 1;
}

/* `<addr6>[,<board4>]` (§10.4 Dw/Ds); Board 0000 ist keine Kennung. */
static uint8_t ziel_lesen(const char *s, uint32_t *addr, uint16_t *board)
{
  size_t n = strlen(s);
  uint32_t b = 0;
  if (n != 6 && n != 11) return 0;
  if (!hexzahl(s, 6, addr)) return 0;
  if (n == 11) {
    if (s[6] != ',' || !hexzahl(s + 7, 4, &b) || b == 0) return 0;
  }
  *board = (uint16_t)b;
  return 1;
}

static eintrag_t *finden(uint32_t addr)
{
  uint8_t i;
  for (i = 0; i < LISTE_MAX; i++)
    if (liste[i].belegt && liste[i].addr == addr) return &liste[i];
  return 0;
}

static uint8_t neue_seq(void)
{
  seq = (uint8_t)((seq + 1) & 0x7F);
  if (!seq) seq = 1;
  return seq;
}

static void funk_stellen(uint8_t erzwingen)
{
  uint8_t soll = !empfang ? DUD_FUNK_HMIP : (dbg_an ? DUD_FUNK_DEBUG : DUD_FUNK_BOOT);
  if (soll != funk || erzwingen) dud_hal_funk(soll);
  funk = soll;
}

/* Rahmen bauen und senden. Rueckgabe: 1 hinaus, sonst Fehlername. */
static const char *senden(uint32_t addr, uint8_t typ, uint8_t s,
                          const uint8_t *body, uint8_t n)
{
  uint8_t f[MAX_RAHMEN];
  if (!empfang) return E_ARG;
  if (KOPF + n > MAX_RAHMEN) return E_ARG;
  f[0] = MAGIC;
  f[1] = typ;
  f[2] = (uint8_t)(addr >> 16);
  f[3] = (uint8_t)(addr >> 8);
  f[4] = (uint8_t)addr;
  f[5] = (uint8_t)(s & 0x7F);
  if (n) memcpy(f + KOPF, body, n);
  if (!dud_hal_konto((uint8_t)(KOPF + n))) return E_LOVF;
  if (!dud_hal_tx(f, (uint8_t)(KOPF + n))) return E_TX;
  return 0;
}

/* ---- Empfang ------------------------------------------------------------ */

static void select_scharf(eintrag_t *e);

static void hello(const uint8_t *f, uint8_t n, int8_t rssi, uint8_t lqi)
{
  uint32_t addr = ((uint32_t)f[2] << 16) | ((uint32_t)f[3] << 8) | f[4];
  eintrag_t *e = finden(addr);
  uint32_t jetzt = dud_hal_ms();

  if (!e) {
    uint8_t i, alt = 0;
    for (i = 0; i < LISTE_MAX; i++) {
      if (!liste[i].belegt) { alt = i; break; }
      /* volle Liste: den am laengsten nicht gehoerten verdraengen (§10.4) */
      if (jetzt - liste[i].t > jetzt - liste[alt].t) alt = i;
    }
    e = &liste[alt];
    memset(e, 0, sizeof(*e));
    e->belegt = 1;
    e->addr = addr;
    memcpy(e->lot, f + 9, 6);
    memcpy(e->die, f + 15, 4);
  } else if (memcmp(e->lot, f + 9, 6) || memcmp(e->die, f + 15, 4)) {
    /* zwei Bausteine unter einer Adresse (§3 HELLO) */
    if (e->koll < 255) e->koll++;
  }

  e->ver   = f[6];
  e->caps  = f[7];
  e->id    = f[8];
  memcpy(e->lot, f + 9, 6);
  memcpy(e->die, f + 15, 4);
  memcpy(e->sig, f + 19, 3);
  e->pg    = (uint16_t)(f[22] | (f[23] << 8));
  e->app   = (uint32_t)f[24] | ((uint32_t)f[25] << 8)
           | ((uint32_t)f[26] << 16) | ((uint32_t)f[27] << 24);
  e->board = (uint16_t)((f[28] << 8) | f[29]);
  e->flags = f[30];
  e->rssi  = rssi;
  e->lqi   = lqi;
  e->t     = jetzt;
  if (e->n < 65535) e->n++;
  if (n >= HELLO_BASIS + DIAG_LEN) {
    memcpy(e->diag, f + HELLO_BASIS, DIAG_LEN);
    e->hat_diag = 1;
  }

  if (addr == w_addr) {
    if (w_zustand == W_ARMED) {
      select_scharf(e);
    } else if (w_zustand == W_SENT && jetzt - w_t >= SELECT_FRIST_MS) {
      /* das Geraet beacont weiter: SELECT verloren, neuer Versuch (§10.4) */
      w_zustand = W_ARMED;
      select_scharf(e);
    }
  }
}

static void print_zeile(const uint8_t *f, uint8_t n)
{
  uint32_t addr = ((uint32_t)f[2] << 16) | ((uint32_t)f[3] << 8) | f[4];
  uint8_t i;
  if (dbg_filter && addr != dbg_filter) return;
  o_f(FS("Dp %06lX "), (unsigned long)addr);
  for (i = KOPF; i < n && i < KOPF + 52; i++)
    dud_hal_putc((f[i] >= 0x20 && f[i] <= 0x7E) ? (char)f[i] : '.');
  dud_hal_zeilenende();
}

/* Einen Rahmen abholen und verteilen; 1 = es kam einer. */
static uint8_t empfangen(void)
{
  uint8_t f[MAX_RAHMEN + 4];
  int8_t rssi = 0;
  uint8_t lqi = 0, n;
  uint32_t addr;

  if (funk == DUD_FUNK_HMIP) return 0;
  n = dud_hal_rx(f, &rssi, &lqi);
  if (!n) return 0;
  if (n < KOPF || f[0] != MAGIC) return 1;
  addr = ((uint32_t)f[2] << 16) | ((uint32_t)f[3] << 8) | f[4];

  if (funk == DUD_FUNK_DEBUG) {
    if (f[1] == T_PRINT) print_zeile(f, n);
    return 1;
  }

  if (f[1] == T_HELLO) {
    if (n >= HELLO_BASIS) hello(f, n, rssi, lqi);
    return 1;
  }

  /* Antwort auf das scharfgestellte SELECT */
  if (w_zustand == W_SENT && f[1] == T_STATUS && addr == w_addr
      && (f[5] & 0x7F) == w_seq && n > KOPF) {
    w_st = f[KOPF];
    w_zustand = w_st ? W_FAIL : W_OK;
    return 1;
  }

  /* Antwort auf eine laufende Anfrage: Typ UND Sequenznummer (§10.8) */
  if (warte_typ && f[1] == warte_typ && addr == warte_addr
      && (f[5] & 0x7F) == warte_seq && !antwort_da) {
    memcpy(antwort, f, n);
    antwort_n = n;
    antwort_da = 1;
  }
  return 1;
}

static void select_scharf(eintrag_t *e)
{
  uint8_t body[3];
  if (e->koll) {
    w_zustand = W_FAIL; w_st = ST_KOLLISION; w_fail_koll = e->koll;
    return;
  }
  if (w_board && e->board != w_board) {
    w_zustand = W_FAIL; w_st = ST_BOARD; w_fail_board = e->board;
    return;
  }
  ziel = e->addr;
  ziel_pg = e->pg;
  w_seq = neue_seq();
  body[0] = w_seq; body[1] = 10; body[2] = 0;       /* {seq, 10 s, Flags} */
  if (senden(e->addr, T_SELECT, w_seq, body, 3) == 0) {
    w_t = dud_hal_ms();
    w_zustand = W_SENT;
  }
}

/*
 * Anfrage an das Sitzungsziel und auf die Antwort warten.
 * Rueckgabe 0 = Antwort liegt in `antwort`, sonst Fehlername.
 */
static const char *anfrage_ex(uint8_t typ, uint8_t *body, uint8_t n,
                              uint8_t antworttyp, uint16_t frist_ms, uint8_t seq_vorne)
{
  const char *e;
  uint8_t s = neue_seq();
  uint32_t t0;

  if (dbg_an) return E_TIMEOUT;           /* Antworten sind nicht hoerbar (§10.4 Db) */
  while (empfangen()) ;                   /* Liegengebliebenes zuerst verteilen */
  if (seq_vorne) body[0] = s;             /* SELECT traegt seine Nummer auch im Body */
  warte_typ = antworttyp; warte_seq = s; warte_addr = ziel; antwort_da = 0;
  e = senden(ziel, typ, s, body, n);
  if (e) { warte_typ = 0; return e; }
  t0 = dud_hal_ms();
  while (dud_hal_ms() - t0 < frist_ms) {
    if (!empfangen()) dud_hal_warten();
    if (antwort_da) { warte_typ = 0; return 0; }
  }
  warte_typ = 0;
  return E_TIMEOUT;
}

static const char *anfrage(uint8_t typ, uint8_t *body, uint8_t n,
                           uint8_t antworttyp, uint16_t frist_ms)
{
  return anfrage_ex(typ, body, n, antworttyp, frist_ms, 0);
}

/* ---- Befehle ------------------------------------------------------------ */

static void cmd_liste(void)
{
  uint8_t i, zahl = 0;
  uint32_t jetzt = dud_hal_ms();
  for (i = 0; i < LISTE_MAX; i++) zahl = (uint8_t)(zahl + liste[i].belegt);
  o_z(FS("D %u"), zahl);
  for (i = 0; i < LISTE_MAX; i++) {
    eintrag_t *e = &liste[i];
    char lot[7];
    char fz;
    uint32_t alter;
    uint8_t k;
    if (!e->belegt) continue;
    if (e->id == 0x00) {
      for (k = 0; k < 6; k++)
        lot[k] = (e->lot[k] >= 0x21 && e->lot[k] <= 0x7E) ? (char)e->lot[k] : '.';
      lot[6] = 0;
    } else {
      lot[0] = '-'; lot[1] = 0;
    }
    if (!(e->flags & 0x01))      fz = '-';
    else if (e->flags & 0x08)    fz = 'B';
    else if (e->flags & 0x02)    fz = 'W';
    else if (e->flags & 0x04)    fz = 'C';
    else                         fz = 'A';
    alter = (jetzt - e->t) / 100;
    o_f(FS("D %06lX b=%04X s=%02X%02X%02X v=%02X c=%02X id=%02X lot=%r "
           "die=%02X%02X%02X%02X pg=%u app=%lu %c rssi=%d lqi=%u age=%lu.%lus n=%u"),
                 (unsigned long)e->addr, e->board, e->sig[0], e->sig[1], e->sig[2],
                 e->ver, e->caps, e->id, lot,
                 e->die[0], e->die[1], e->die[2], e->die[3],
                 e->pg, (unsigned long)e->app, fz, e->rssi, e->lqi,
                 (unsigned long)(alter / 10), (unsigned long)(alter % 10), e->n);
    if (e->flags & 0x08)
      o_f(FS(" btn=%umin"), e->flags >> 4);
    if (e->koll)
      o_f(FS(" KOLLISION=%u"), e->koll);
    dud_hal_zeilenende();
    if (e->hat_diag) {
      const uint8_t *d = e->diag;
      /* Reihenfolge der Diagnosebytes wie im Bootloader (§10.4 Dl) */
      o_z(FS("D %06lX rx sy=%u ok=%u da=%u ln=%u ty=%02X bl=%u sp=%u cs=%u mc=%02X"),
               (unsigned long)e->addr, d[0], d[1], d[2], d[3], d[13], d[4], d[5], d[6], d[7]);
      o_z(FS("D %06lX ses fr=%u bad=%u cm=%u tx=%u map=%02X rst=%u cp=%u"),
               (unsigned long)e->addr, d[8], d[9], d[10], d[11], d[12], d[14], d[15]);
    }
  }
}

static void cmd_scharf(const char *rest)
{
  uint32_t addr;
  uint16_t board;

  if (rest[0] == 'x') {                     /* Dwx: still (§10.4) */
    w_zustand = W_IDLE; w_addr = 0; w_board = 0;
    return;
  }
  if (!rest[0]) {                           /* Dw: Stand */
    o_f(FS("Dw %06lX %s st=%u pg=%u"),
                 (unsigned long)w_addr, w_name(w_zustand), w_st, ziel_pg);
    if (w_zustand == W_FAIL && w_st == ST_BOARD)
      o_f(FS(" BOARD b=%04X want=%04X"), w_fail_board, w_board);
    else if (w_zustand == W_FAIL && w_st == ST_KOLLISION)
      o_f(FS(" KOLLISION n=%u"), w_fail_koll);
    dud_hal_zeilenende();
    return;
  }
  if (!ziel_lesen(rest, &addr, &board)) {
    o_z(FS("Dw ERR"));
    return;
  }
  if (!empfang) { empfang = 1; funk_stellen(0); }
  w_addr = addr;
  w_board = board;
  w_st = ST_KEINER;
  w_zustand = W_ARMED;
  if (board)
    o_z(FS("Dw %06lX armed want=%04X"), (unsigned long)addr, board);
  else
    o_z(FS("Dw %06lX armed"), (unsigned long)addr);
}

static void cmd_sitzung(const char *rest)
{
  uint32_t addr;
  uint16_t board;
  eintrag_t *e;
  const char *err = E_TIMEOUT;
  uint8_t i;

  if (!ziel_lesen(rest, &addr, &board)) {
    o_z(FS("Ds ERR"));
    return;
  }
  ziel_board = board;
  e = finden(addr);

  if (e && e->koll) {                               /* 1. Kollision */
    o_z(FS("Ds %06lX KOLLISION n=%u st=177 \xE2\x80\x94 Dc, dann neu lauschen"),
             (unsigned long)addr, e->koll);
    return;
  }
  if (board && (!e || e->board != board)) {         /* 2. Board-Tor */
    o_z(FS("Ds %06lX BOARD b=%04X want=%04X st=176"),
             (unsigned long)addr, e ? e->board : 0, board);
    return;
  }
  ziel = addr;
  if (e) ziel_pg = e->pg;

  if (w_zustand == W_OK && w_addr == addr) {        /* 3. Sitzung steht schon */
    w_zustand = W_IDLE;
    o_z(FS("Ds %06lX OK st=0 pg=%u"),
             (unsigned long)addr, ziel_pg);
    return;
  }

  for (i = 0; i < SELECT_ANLAEUFE; i++) {           /* 4. blind */
    uint8_t body[3];
    body[0] = 0; body[1] = 10; body[2] = 0;         /* {seq, 10 s, Flags} */
    err = anfrage_ex(T_SELECT, body, 3, T_STATUS, SELECT_FRIST_MS, 1);
    if (!err) {
      o_z(FS("Ds %06lX OK st=%u pg=%u"),
               (unsigned long)addr, antwort[KOPF], ziel_pg);
      return;
    }
    if (err == E_ARG || err == E_LOVF) break;
  }
  o_z(FS("Ds %06lX %s st=255 pg=%u"),
           (unsigned long)addr, err, ziel_pg);
}

static void cmd_erase(const char *rest)
{
  uint32_t seite;
  uint8_t body[2], i;
  const char *err = E_TIMEOUT;
  size_t n = strlen(rest);

  if (!hexzahl(rest, (uint8_t)(n > 8 ? 9 : n), &seite) || seite > 0xFFFF) {
    o_z(FS("De ERR"));
    return;
  }
  body[0] = (uint8_t)seite; body[1] = (uint8_t)(seite >> 8);
  for (i = 0; i < 2; i++) {
    err = anfrage(T_ERASE, body, 2, T_STATUS, ERASE_FRIST_MS);
    if (!err) {
      o_z(FS("De %04lX OK st=%u"),
               (unsigned long)seite, antwort[KOPF]);
      return;
    }
    if (err == E_ARG || err == E_LOVF) break;
  }
  o_z(FS("De %04lX %s st=255"), (unsigned long)seite, err);
}

static void cmd_puffer(const char *rest)
{
  uint32_t nr;
  size_t n = strlen(rest);
  if (n > 4 || !hexzahl(rest, (uint8_t)n, &nr)) {
    o_z(FS("Dp ERR"));
    return;
  }
  puf_nr = (uint16_t)nr;
  puf_map = 0;
  memset(puf, 0xFF, sizeof puf);
}

static void cmd_fragment(const char *rest)
{
  int8_t f = hexziffer(rest[0]);
  const char *hex = rest + 2;
  size_t n = strlen(rest) >= 2 ? strlen(hex) : 0;
  uint16_t off, soll, i;

  if (f < 0 || rest[1] != ',' || n < 2 || n > 98 || (n & 1) || f >= 8
      || puf_nr == 0xFFFF)
    goto fehler;
  off = (uint16_t)(f * FRAG);
  if (off >= ziel_pg || ziel_pg > SEITE_MAX) goto fehler;
  soll = (uint16_t)((ziel_pg - off) < FRAG ? (ziel_pg - off) : FRAG);
  if (n / 2 != soll) goto fehler;
  for (i = 0; i < soll; i++) {
    int8_t h = hexziffer(hex[2 * i]), l = hexziffer(hex[2 * i + 1]);
    if (h < 0 || l < 0) goto fehler;
  }
  for (i = 0; i < soll; i++)
    puf[off + i] = (uint8_t)((hexziffer(hex[2 * i]) << 4) | hexziffer(hex[2 * i + 1]));
  puf_map |= (uint8_t)(1u << f);
  return;
fehler:
  o_z(FS("Dd ERR"));
}

static void dm_ausgabe(uint16_t nr, const char *res, uint8_t st, uint8_t map,
                       uint8_t runden, const uint8_t *diag, uint8_t diag_n, uint16_t want)
{
  uint8_t x[24];
  memset(x, 0xFF, sizeof x);
  if (diag_n > 24) diag_n = 24;
  if (diag_n) memcpy(x, diag, diag_n);
  o_z(FS("Dm %04X %s st=%u map=%02X r=%u sy=%u ok=%u da=%u ln=%u"),
           nr, res, st, map, runden, x[0], x[1], x[2], x[3]);
  o_z(FS("Dm crc want=%04X have=%02X%02X buf=%02X%02X..%02X%02X"),
           want, x[5], x[4], x[6], x[7], x[8], x[9]);
  o_z(FS("Dm frag %02X %02X %02X %02X %02X %02X"),
           x[10], x[11], x[12], x[13], x[14], x[15]);
  o_z(FS("Dm n= %u %u %u %u %u %u  rst=%u cm=%u"),
           x[16], x[17], x[18], x[19], x[20], x[21], x[22], x[23]);
}

static void cmd_seite(void)
{
  uint16_t nr = puf_nr, pg = ziel_pg, want;
  uint8_t frags, voll, fehlt, runde, i;
  uint8_t st = ST_KEINER, map = 0, beantwortet = 0;
  uint8_t diag[24];
  uint8_t diag_n = 0;
  uint16_t frist;
  const char *err = 0;

  if (nr == 0xFFFF) {
    dm_ausgabe(0xFFFF, E_STATE, ST_KEINER, 0, 0, 0, 0, 0);
    return;
  }
  if (pg == 0 || pg > SEITE_MAX) pg = SEITE_MAX;
  frags = (uint8_t)((pg + FRAG - 1) / FRAG);
  voll = (uint8_t)((1u << frags) - 1);
  if (puf_map != voll) {                      /* unvollstaendig: Puffer bleibt offen */
    dm_ausgabe(nr, E_STATE, ST_KEINER, puf_map, 0, 0, 0, 0);
    return;
  }

  /* Ab hier ist der Puffer verbraucht, wie immer es ausgeht (§10.8). */
  puf_nr = 0xFFFF;
  puf_map = 0;
  want = crc16(puf, pg);
  frist = (nr & PAGE_EE) ? COMMIT_EE_MS : COMMIT_FRIST_MS;

  if (!empfang) {
    dm_ausgabe(nr, E_ARG, ST_KEINER, 0, 1, 0, 0, want);
    return;
  }

  fehlt = voll;
  for (runde = 1; runde <= 4; runde++) {
    uint8_t body[3 + FRAG], commit[4];
    for (i = 0; i < frags; i++) {
      uint16_t off = (uint16_t)(i * FRAG);
      uint8_t n = (uint8_t)((pg - off) < FRAG ? (pg - off) : FRAG);
      uint32_t t0;
      if (!(fehlt & (1u << i))) continue;
      body[0] = (uint8_t)nr; body[1] = (uint8_t)(nr >> 8); body[2] = i;
      memcpy(body + 3, puf + off, n);
      err = senden(ziel, T_DATA, neue_seq(), body, (uint8_t)(3 + n));
      if (err) break;
      t0 = dud_hal_ms();
      while (dud_hal_ms() - t0 < DATA_ABSTAND_MS) { if (!empfangen()) dud_hal_warten(); }
    }
    if (err && err != E_TX) {                 /* Konto leer oder Empfang aus: aufgeben */
      dm_ausgabe(nr, err, st, map, runde, diag, diag_n, want);
      return;
    }
    commit[0] = (uint8_t)nr; commit[1] = (uint8_t)(nr >> 8);
    commit[2] = (uint8_t)want; commit[3] = (uint8_t)(want >> 8);
    err = anfrage(T_COMMIT, commit, 4, T_COMMIT_ACK, frist);
    if (err == E_ARG || err == E_LOVF) {
      dm_ausgabe(nr, err, st, map, runde, diag, diag_n, want);
      return;
    }
    if (err || antwort_n < KOPF + 4) {        /* keine Antwort: ganze Runde nochmal */
      fehlt = voll;
      beantwortet = 0;
      continue;
    }
    beantwortet = 1;
    map = antwort[KOPF + 2];
    st  = antwort[KOPF + 3];
    diag_n = (uint8_t)(antwort_n - (KOPF + 4));
    if (diag_n > sizeof diag) diag_n = sizeof diag;
    memcpy(diag, antwort + KOPF + 4, diag_n);
    if (st == 0) {
      dm_ausgabe(nr, FS("OK"), st, map, runde, diag, diag_n, want);
      return;
    }
    fehlt = (uint8_t)(voll & ~map);
    if (!fehlt) fehlt = voll;
  }
  dm_ausgabe(nr, beantwortet ? FS("OK") : E_TIMEOUT, st, map, 5, diag, diag_n, want);
}

static void cmd_pruefen(const char *rest, uint8_t eeprom)
{
  const char *komma = strchr(rest, ',');
  uint32_t von, len;
  uint8_t body[8], i;
  const char *err = E_TIMEOUT;
  const char *name = eeprom ? FS("DV") : FS("Dv");

  if (!komma || komma == rest || !komma[1]
      || !hexzahl(rest, (uint8_t)((komma - rest) > 8 ? 9 : (komma - rest)), &von)
      || !hexzahl(komma + 1, (uint8_t)(strlen(komma + 1) > 8 ? 9 : strlen(komma + 1)), &len)) {
    o_z(FS("%s ERR"), name);
    return;
  }
  for (i = 0; i < 4; i++) {
    body[i]     = (uint8_t)(von >> (8 * i));
    body[4 + i] = (uint8_t)(len >> (8 * i));
  }
  for (i = 0; i < 2; i++) {
    err = anfrage(eeprom ? T_EE_VERIFY : T_VERIFY, body, 8, T_VERIFY_ACK, VERIFY_FRIST_MS);
    if (!err && antwort_n >= KOPF + 3) {
      o_z(FS("%s %04X OK st=%u"), name,
               (uint16_t)(antwort[KOPF] | (antwort[KOPF + 1] << 8)), antwort[KOPF + 2]);
      return;
    }
    if (err == E_ARG || err == E_LOVF) break;
    if (!err) err = E_TIMEOUT;                /* Antwort zu kurz */
  }
  o_z(FS("%s 0000 %s st=255"), name, err);
}

static void cmd_go(void)
{
  uint8_t i;
  const char *err = E_TIMEOUT;
  for (i = 0; i < 3; i++) {
    err = anfrage(T_GO, 0, 0, T_STATUS, GO_FRIST_MS);
    if (!err) {
      o_z(FS("Dg OK st=%u"), antwort[KOPF]);
      return;
    }
    if (err == E_ARG || err == E_LOVF) break;
  }
  o_z(FS("Dg %s st=255"), err);
}

static void cmd_debug(const char *rest)
{
  uint32_t filter = 0;
  if (rest[0] == '-') {
    empfang = 1; dbg_an = 0;
    funk_stellen(1);
    o_z(FS("Db aus, zurueck auf 868.4998"));
    return;
  }
  if (strlen(rest) > 8 || !hexzahl(rest, (uint8_t)strlen(rest), &filter)) {
    o_z(FS("Db ERR"));
    return;
  }
  empfang = 1; dbg_an = 1; dbg_filter = filter;
  funk_stellen(1);
  if (filter)
    o_z(FS("Db an, 869.5250 MHz, nur %06lX"), (unsigned long)filter);
  else
    o_z(FS("Db an, 869.5250 MHz, alle Geraete"));
  o_z(FS("Db HINWEIS: solange an, keine Dudette-Sitzung moeglich"));
}

static void cmd_kommando(const char *rest)
{
  uint32_t addr;
  char c;
  uint8_t arg = 0, body[2];
  const char *p;

  if (strlen(rest) < 7 || !hexzahl(rest, 6, &addr)) {
    o_z(FS("Dk ERR"));
    return;
  }
  c = rest[6];
  for (p = rest + 7; *p; p++) {                 /* die letzten zwei Hexziffern zaehlen */
    int8_t d = hexziffer(*p);
    if (d >= 0) arg = (uint8_t)((arg << 4) | (uint8_t)d);
  }
  if (!dbg_an) {
    o_z(FS("Dk %06lX %c %02X %s"), (unsigned long)addr, c, arg, E_STATE);
    return;
  }
  body[0] = (uint8_t)c; body[1] = arg;
  {
    const char *err = senden(addr, T_CMD, neue_seq(), body, 2);
    o_z(FS("Dk %06lX %c %02X %s"),
             (unsigned long)addr, c, arg, err ? err : FS("OK"));
  }
}

/* ---- Schnittstelle nach aussen ------------------------------------------ */

void dud_init(void)
{
  memset(liste, 0, sizeof liste);
  funk = DUD_FUNK_HMIP;
  empfang = 0; dbg_an = 0; dbg_filter = 0;
  w_zustand = W_IDLE; w_addr = 0; w_board = 0; w_st = ST_KEINER;
  ziel = 0; ziel_pg = 256; ziel_board = 0;
  puf_nr = 0xFFFF; puf_map = 0;
  warte_typ = 0;
}

uint8_t dud_funk(void) { return funk; }

void dud_task(void)
{
  uint8_t i;
  uint32_t still;
  for (i = 0; i < 4; i++)
    if (!empfangen()) break;

  if (funk == DUD_FUNK_HMIP) return;
  still = dud_hal_ms() - letzter_befehl;
  if (w_zustand == W_ARMED || w_zustand == W_SENT) return;
  if (still < (dbg_an ? RUECKFALL_DBG_MS : RUECKFALL_MS)) return;
  empfang = 0;
  dbg_an = 0;
  funk_stellen(0);
}

void dud_befehl(const char *z)
{
  char sub;
  const char *rest;

  if (z[0] != 'D') return;
  letzter_befehl = dud_hal_ms();
  sub = z[1];
  rest = sub ? z + 2 : z + 1;

  switch (sub) {
  case 0:   empfang = 0; dbg_an = 0; funk_stellen(0); return;            /* D  */
  case 'x': empfang = 0; dbg_an = 0; funk_stellen(0); return;            /* Dx */
  case 'r': empfang = 1; dbg_an = 0; funk_stellen(1); return;            /* Dr, A7 */
  case 'c': memset(liste, 0, sizeof liste); return;                      /* Dc */
  case 'l': cmd_liste(); return;
  case 'w': cmd_scharf(rest); return;
  case 's': cmd_sitzung(rest); return;
  case 'e': cmd_erase(rest); return;
  case 'p': cmd_puffer(rest); return;
  case 'd': cmd_fragment(rest); return;
  case 'm': cmd_seite(); return;
  case 'v': cmd_pruefen(rest, 0); return;
  case 'V': cmd_pruefen(rest, 1); return;
  case 'g': cmd_go(); return;
  case 'b': cmd_debug(rest); return;
  case 'k': cmd_kommando(rest); return;
  default:
    o_z(FS("D ERR"));
    return;
  }
}
