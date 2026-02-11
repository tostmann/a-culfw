# CULFW32 Erweiterte Kommando-Referenz

Diese Dokumentation beschreibt die neuen und erweiterten Kommandos der 32-Bit Portierung (ESP32/STM32) der `a-culfw`. Für die Standard-Kommandos siehe [commandref.md](docs/commandref.md).

## Erweiterte Standard-Kommandos

### `V` - Version anzeigen
Gibt die Firmware-Version, das Build-Datum, den Board-Typ und die erkannte Standard-Frequenz aus.
*   **Beispiel:** `V 1.26.274 a-culfw Build: 274 (2026-02-12) XIAO-ESP32-C6 (F-Band: 868MHz)`
*   **Besonderheit:** Zeigt an, ob das Board via Hardware-Bootstrap (`MARK433_PIN`) als 433MHz oder 868MHz erkannt wurde.

### `e` - EEPROM Factory Reset
Löscht die im emulierten EEPROM (Flash) gespeicherten Einstellungen und führt einen Neustart des Mikrocontrollers durch.
*   **ESP32:** Nutzt die Arduino-EEPROM Bibliothek zur Persistenz.

### `P` - Pin / LED Diagnose
Gibt den Status interner Zustände in Hexadezimalform aus. Dient primär der Entwicklerdiagnose.

---

## Diagnose-Kommando `y` (Erweitert)

Das Kommando `y` wurde massiv erweitert, um die RF-Performance und den Systemstatus in Echtzeit zu überwachen.

**Ausgabeformat:**
`Debug: Ticks: <T> ISR: <I> H/L: <H>/<L> ST: <S> SY: <Y> Z: <Z_H>/<Z_L> O: <O_H>/<O_L> F:<F> I433:<I433> B:<B> ANT:<A> SCAN:<SC>`

| Parameter | Beschreibung |
| :--- | :--- |
| **Ticks** | System-Heartbeat (125Hz). |
| **ISR** | Anzahl der GDO0-Interrupts (Flankenwechsel) seit dem letzten Reset. |
| **H/L** | Aktuelle High- und Low-Puls-Dauer (in µs) der letzten Flanke. |
| **ST** | Interner State-Machine Status der SlowRF-Erkennung (`0=IDLE`, `1=SYNC`, `2=COLLECT`). |
| **SY** | Sync-Bit Status. Zeigt an, ob eine gültige Sync-Sequenz erkannt wurde. |
| **Z** | (Zero-Wave) Erkannte Timing-Durchschnittswerte für ein '0' Bit (High/Low in µs). |
| **O** | (One-Wave) Erkannte Timing-Durchschnittswerte für ein '1' Bit (High/Low in µs). |
| **F** | Aktueller Frequenzmodus (`0=Unset`, `1=433MHz`, `2=868MHz`). |
| **I433** | Status des `IS433MHZ` Makros (0 oder 1). |
| **B** | Füllstand der RF-Empfangspuffer (`bucket_nrused`). |
| **ANT** | Status des Antennenschalters (nur XIAO-ESP32-C6, `0=Onboard`, `1=External`). |
| **SCAN** | Toggle-Monitor. Zeigt für alle GPIOs an, ob Aktivität erkannt wurde (Hex-Bitmaske). |

---

## RF & Protokoll Spezifikationen

### Sende-Modus (TX)
Bei Protokollen wie FS20 (`F`), Intertechno (`i`) oder Somfy (`Y`) schaltet der ESP32 die FreeRTOS-Task-Priorität kurzzeitig auf das Maximum (`configMAX_PRIORITIES - 1`), um Jitter beim Bit-Banging zu minimieren.

### Hardware-Auto-Detektion
Die Firmware unterstützt eine automatische Hardware-Erkennung beim Booten:
*   **868 MHz (Default):** Interner Pull-up am `MARK433_PIN` ist aktiv.
*   **433 MHz:** Wenn `MARK433_PIN` (z.B. GPIO 4) beim Start gegen GND gezogen ist (10k Widerstand), werden automatisch 433 MHz Register geladen.

---

## Debugging-Tools (Python)

Im Verzeichnis `tools/` befinden sich Skripte zur tieferen Analyse:
*   `check_v.py`: Schnelle Abfrage von Version und Debug-Status.
*   `test_fs20_tx.py`: Sendet FS20 Testpakete.
*   `test_it_tx.py`: Sendet Intertechno Testpakete.
*   `analyze_pulses.py`: Analysiert die Roh-Pulszeiten des CC1101 GDO0.
