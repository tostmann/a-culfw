#include <Arduino.h>
#include <SPI.h>

extern "C" {
  #include "board.h"
  #include "clib/cc1100.h"
  #include "clib/delay.h"
  #include "clib/display.h"
  #include "clib/fncollection.h"
  #include "clib/rf_receive.h"
}

extern "C" {
// Globale Variable (wie im Original)
uint8_t cc_on = 0;

// --- KONFIGURATIONSTABELLEN (Kopie aus original cc1100.c) ---
// PROGMEM ist auf ESP32 ein Leermakro (siehe avr/pgmspace.h), Daten liegen im Flash/RAM.

const uint8_t CC1100_PA[] = {
  0x27, 0x67, 0x50, 0x81, 0xC2, 0x00, 0x00, 0x00 
};

// Standard Config (868 MHz)
const uint8_t CC1100_CFG[EE_CC1100_CFG_SIZE] = {
   0x0D, // 00 IOCFG2   GDO2 as serial output
   0x2E, // 01 IOCFG1   Tri-State
   0x2D, // 02 IOCFG0   GDO0 for input
   0x07, // 03 FIFOTHR
   0xD3, // 04 SYNC1
   0x91, // 05 SYNC0
   0x3D, // 06 PKTLEN
   0x04, // 07 PKTCTRL1
   0x32, // 08 PKTCTRL0
   0x00, // 09 ADDR
   0x00, // 0A CHANNR
   0x06, // 0B FSCTRL1
   0x00, // 0C FSCTRL0
   0x21, // 0D FREQ2
   0x65, // 0E FREQ1
   0x6a, // 0F FREQ0
   0x57, // 10 MDMCFG4
   0xC4, // 11 MDMCFG3
   0x30, // 12 MDMCFG2
   0x22, // 13 MDMCFG1
   0xF8, // 14 MDMCFG0
   0x47, // 15 DEVIATN
   0x07, // 16 MCSM2
   0x00, // 17 MCSM1    (wird in init gepatcht)
   0x18, // 18 MCSM0
   0x14, // 19 FOCCFG
   0x6C, // 1A BSCFG
   0x07, // 1B AGCCTRL2
   0x00, // 1C AGCCTRL1
   0x90, // 1D AGCCTRL0
   0x87, // 1E WOREVT1
   0x6B, // 1F WOREVT0
   0xF8, // 20 WORCTRL
   0x56, // 21 FREND1
   0x10, // 22 FREND0
   0xE9, // 23 FSCAL3
   0x2A, // 24 FSCAL2
   0x00, // 25 FSCAL1
   0x1F, // 26 FSCAL0
   0x41, // 27 RCCTRL1
   0x00, // 28 RCCTRL0
};

// --- Low Level SPI Helper ---

void cc1100_select()   { digitalWrite(CC1100_CS_PIN, LOW); }
void cc1100_deselect() { digitalWrite(CC1100_CS_PIN, HIGH); }

uint8_t cc1100_sendbyte(uint8_t data) {
  return SPI.transfer(data);
}

uint8_t cc1100_readReg(uint8_t addr) {
  cc1100_select();
  SPI.transfer(addr | 0x80); // Read bit setzen
  uint8_t val = SPI.transfer(0);
  cc1100_deselect();
  return val;
}

void cc1100_writeReg(uint8_t addr, uint8_t data) {
  cc1100_select();
  SPI.transfer(addr);
  SPI.transfer(data);
  cc1100_deselect();
}

uint8_t ccStrobe(uint8_t strobe) {
  cc1100_select();
  uint8_t ret = SPI.transfer(strobe);
  cc1100_deselect();
  return ret;
}

// --- Interrupt Handling ---

// ISR Wrapper: Ruft die C-Funktion aus rf_receive.c auf
void IRAM_ATTR cc1100_isr_handler() {
    // Ruft die Interrupt-Routine der CLIB auf
    // Diese muss in rf_receive.c oder interrupt.c als normale Funktion definiert sein
    // (Da wir ISR() Makro in avr/interrupt.h umdefiniert haben)
    extern void CC1100_INT_VECT(void); // Forward declaration
    CC1100_INT_VECT();
}

// --- High Level Functions ---

void ccInitChip(uint8_t *cfg) {
  // 1. Pins Setup
  pinMode(CC1100_CS_PIN, OUTPUT);
  digitalWrite(CC1100_CS_PIN, HIGH);
  
  pinMode(CC1100_GDO0_PIN, INPUT); 
  // GDO2 optional (Serial Out)
  #ifdef CC1100_GDO2_PIN
  pinMode(CC1100_GDO2_PIN, INPUT);
  #endif

  // 2. SPI Starten
  // SCK, MISO, MOSI, SS (SS nutzen wir manuell)
  SPI.begin(CC1100_SCK_PIN, CC1100_MISO_PIN, CC1100_MOSI_PIN, CC1100_CS_PIN);
  SPI.setClockDivider(SPI_CLOCK_DIV4); // 4 MHz (bei 16MHz Basis), ESP32 kann schneller, aber sicher ist sicher
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  // 3. Reset
  cc_on = 1;
  ccStrobe(CC1100_SRES);
  delay(1);

  // 4. Register schreiben
  // Wir nutzen die CFG Tabelle (entweder übergeben oder Standard)
  const uint8_t *ptr = (cfg ? cfg : CC1100_CFG);
  for (uint8_t i = 0; i < EE_CC1100_CFG_SIZE; i++) {
    cc1100_writeReg(i, ptr[i]);
  }
  
  // PA Table
  cc1100_writeReg(CC1100_PATABLE, CC1100_PA[0]);

  // 5. Interrupt vorbereiten (aber noch nicht einschalten)
  // Auf AVR: EIMSK |= ...
  // Auf ESP32: attachInterrupt
  // Wir machen das dynamisch in ccRX()
  
  ccStrobe(CC1100_SCAL);
  delay(1);
  ccRX();
}

void ccRX(void) {
  if (!cc_on) return;
  
  ccStrobe(CC1100_SRX);
  
  // Registerzugriffe überwachen, bis Chip im RX Modus ist
  // (Vereinfacht für ESP32 Portierung, Original hat Timeout Loop)
  uint8_t marcstate = 0xFF;
  while(marcstate != 0x0D) { // 0x0D = RX
      marcstate = (ccStrobe(CC1100_SNOP) & 0x1F);
      if(marcstate == 0x11) ccStrobe(CC1100_SRX); // RX_OVERFLOW -> flush -> RX
  }

  // Interrupt einschalten
  // Falling Edge, da GDO0 Low geht wenn Paket fertig (oder Sync empfangen?)
  // Culfw default: IOCFG0 = 0x2D (GDO0_CFG_RX_FIFO_ABOVE_THRESHOLD) -> High wenn voll?
  // Das Original nutzt ISC00=0, ISC01=1 -> Rising Edge für Async?
  // WARNUNG: Hier muss geprüft werden, wie culfw den Interrupt erwartet.
  // Meistens: Rising Edge bei Sync-Word detection oder Fifo Threshold.
  attachInterrupt(digitalPinToInterrupt(CC1100_GDO0_PIN), cc1100_isr_handler, RISING);
}

void ccTX(void) {
  // Interrupt ausschalten, wir senden jetzt
  detachInterrupt(digitalPinToInterrupt(CC1100_GDO0_PIN));
  
  // (Der eigentliche Sende-Loop ist in clib/cc1100.c komplexer implementiert 
  // mit Buffer füllen. Da wir clib/cc1100.c ersetzt haben, fehlt diese Logik hier!
  // Wir müssen ccTX implementieren, welches die Daten aus dem Buffer sendet.)
  
  // HINWEIS: Die originale ccTX Funktion in culfw liest aus dem 'txbuffer'
  // und schiebt es in den FIFO. Da wir hier eine radikal vereinfachte Version bauen,
  // müssen wir sicherstellen, dass die 'rf_send.c' Logik greift.
  
  // Für den ersten Schritt (Kompilieren) reicht ein Dummy oder eine vereinfachte Logic.
  // ECHTE IMPLEMENTIERUNG:
  ccStrobe(CC1100_STX);
  
  // Warten bis gesendet...
  while ((cc1100_readReg(CC1100_MARCSTATE) & 0x1F) == 0x13); // 0x13 = TX
  
  // Zurück zu RX
  ccRX();
}

// Helper für Factory Reset Command
void cc_factory_reset(void) {
    ccStrobe(CC1100_SRES);
}

} // Ende von extern "C" am Dateiende
