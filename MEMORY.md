# CULFW32 Project Status

## 1. General Project Information
*   **Project Name:** CULFW32 (Portierung der a-culfw auf 32-Bit-Mikrocontroller)
*   **Repository:** `tostmann/a-culfw.git` (forked)
*   **Local Codebase:** `/opt/ai_builder_data/users/763684262/projects/CULFW32/culfw/`
*   **Build System:** PlatformIO
*   **Target Architectures:** AVR, STM32, ESP32 (RISC-V)

## 2. Supported Boards & Firmware Versions
*   **Firmware Version:** 1.26.82 (latest build, automatisch inkrementiert nach Fixes)
*   **AVR Targets (Verified):**
    *   `CUL_V3.hex`
    *   `nanoCUL868.hex`
    *   `nanoCUL433.hex`
*   **STM32 Targets (Verified & Fixed):**
    *   `MapleCUNx4_W5100_BL.bin`
    *   `MapleCUNx4_W5500_BL.bin`
*   **ESP32 Targets (Stable & Functional):**
    *   **ESP32-C3:**
        *   `ESP32-C3-factory.bin` (Vollständiges Image für `0x0`)
        *   `ESP32-C3.bin` (Nur Applikation für OTA)
        *   **Pinout:** LED=GPIO4, SPI: SCK=6, MISO=5, MOSI=7, CS=10, GDO0=2, GDO2=3.
    *   **ESP32-C6:**
        *   `ESP32-C6-factory.bin` (Vollständiges Image für `0x0`)
        *   `ESP32-C6.bin` (Nur Applikation für OTA)
        *   **Pinout:** LED=GPIO8 (DevKitC-1 Standard), SPI Pins wie in `board.h` definiert.
    *   **XIAO-ESP32-C3:** (MAX!/Moritz Functional, SlowRF in progress)
        *   `XIAO-ESP32-C3-factory.bin` (Vollständiges Image für `0x0`)
        *   `XIAO-ESP32-C3.bin` (Nur Applikation für OTA)
        *   **Angewendetes Pinout (Verifiziert & Funktional):** LED=GPIO21 (Pin D6), SPI: SCK=GPIO6 (Pin D4), MISO=GPIO10 (Pin D10), MOSI=GPIO7 (Pin D5), CS=GPIO5 (Pin D3), GDO0=GPIO3 (Pin D1), GDO2=GPIO4 (Pin D2).

## 3. Key Architectural Decisions & Changes
*   **Build-Prozess (DONE):** Alle ESP32-Targets generieren jetzt automatisch sowohl `factory.bin` (komplettes Image mit Bootloader/Partitionstabelle) als auch `app.bin` (für OTA). Die finalen Binaries werden in das Projekt-Root-Verzeichnis `CULFW32/binaries/` exportiert.
*   **STM32 Linker-Fehler (DONE):** `ticks`-Variable deklariert und `hal_eeprom_init()` in `main.c` hinzugefügt, um Linker-Fehler zu beheben.
*   **`clib` Refactoring (Allgemeine Kompatibilität - DONE):**
    *   `rf_receive_bucket.h` / `rf_receive.c`: `packetCheckValues` zur `extern`-Deklaration verschoben, um "multiple definition"-Fehler zu vermeiden.
    *   `ttydata.h` / `ttydata.c`: Funktionspointer (`input_handle_func`) verwenden nun `extern` für korrekte Verlinkung.
    *   `display.c`: USB-Verbindungsstatuslogik auf ESP32 erweitert.
    *   `clock.c`: `ticks`-Variable für `USE_HAL`-Plattformen konditional kompiliert.
    *   `rf_receive.c`: CC1101-zeitkritische Routinen um `defined(ESP32) || defined(STM32)` erweitert.
    *   `fncollection.c`: Null-Pointer-Check in `eeprom_factory_reset`, `hal_eeprom_init()` für HAL-Plattformen integriert.
*   **ESP32 USB-CDC & Konsole (DONE - Initiales Setup. Boot-Probleme durch Strapping-Pins behoben):**
    *   `ARDUINO_USB_MODE=1` und `ARDUINO_USB_CDC_ON_BOOT=1` für direkte USB-Serial über `Serial`-Objekt aktiviert.
    *   Doppelte Initialisierungen im `setup()` wurden entfernt.
    *   `Serial` wird konditional zu `USBSerial` definiert, um sicherzustellen, dass die USB-CDC-Schnittstelle verwendet wird.
    *   `Serial.setTxTimeoutMs(0)` wurde für nicht-blockierende serielle Ausgaben hinzugefügt.
    *   Explizite Nutzung von `USBSerial` im Minimal-Sketch für Debugging versucht, jedoch ohne Erfolg bei der Ausgabe, bis das Pinning korrigiert wurde.
*   **ESP32 Pointer-Handling Fix (DONE):** `ttydata.c` korrigiert, um 32-Bit-Funktionspointer auf ESP32/ARM korrekt zu behandeln (Behebung des `Instruction access fault`-Absturzes, verursacht durch 16-Bit-AVR-Makro `__LPM_word` beim Auslesen der `fntab`).
*   **Flexible Pin-Konfiguration (DONE):** `board.h` (für ESP32) verwendet jetzt `#ifndef`-Guards, wodurch Pin-Definitionen über `build_flags` in `platformio.ini` für spezifische Board-Targets überschrieben werden können.
*   **PlatformIO Build-System-Fixes & Pfad-Standardisierung für ESP32-Targets (DONE):**
    *   Die globale `src_dir = .` Einstellung wurde in die `[platformio]` Sektion der Haupt-`platformio.ini` (`culfw/platformio.ini`) verschoben, um das Projekt-Root-Verzeichnis als Quellverzeichnis für alle Targets zu definieren.
    *   Alle `build_flags` für Include-Pfade (`-I`) und `build_src_filter` in `culfw/Devices/ESP32/platformio.ini` wurden für die ESP32-Targets (ESP32-C3, XIAO-ESP32-C3, ESP32-C6) auf korrekte relative Pfade zum Projekt-Root (`./`, `clib`, `ESP32` etc.) angepasst.
    *   Dies ermöglicht nun den erfolgreichen Build aller ESP32-Targets (`ESP32-C3`, `XIAO-ESP32-C3`, `ESP32-C6`) aus dem Projekt-Root (`culfw/`) mit dem Kommando `pio run -d culfw/ -e <env_name>`.
    *   Alle ESP32-Targets bauen nun erfolgreich und generieren sowohl `factory.bin` als auch `app.bin`.
*   **AVR-spezifische Header-Dateien für ESP32 (DONE):** Die `#include <avr/io.h>` und `#include <avr/pgmspace.h>` Pfade wurden in den `build_flags` für ESP32-Targets korrekt referenziert, um Kompilierungsfehler zu beheben (Warnung `_BV` Redefinition bleibt).
*   **GDO-Interrupt-Handling & Debugging für ESP32 (DONE - Code-Implementierung, VERIFIED nach Boot-Fix, jedoch neue WDT-Timeout-Probleme bei SlowRF):**
    *   **Problemursache (ehemals):** Der GDO0-Interrupt wurde in `hal_CC_GDO_init()` zu früh aktiviert, was zu einem "Interrupt Storm" führte.
    *   **Fix (Code):** Der Interrupt wird nun explizit nach vollständiger CC1101-Initialisierung in `setup()` aktiviert. `pinMode(GDO0_PIN, INPUT_PULLUP)` und `IRAM_ATTR` für den ISR-Handler wurden gesetzt. Der `gdo_isr_count` zur Überwachung der Interrupt-Aktivität wurde implementiert.
    *   **Neue Beobachtung:** Nach Umstellung von `IOCFG0` auf `0x0D` (Serial Data Output) für SlowRF, steigt der `gdo_isr_count` massiv an, führt aber zu einem Interrupt Watchdog Timeout auf CPU0. Dies deutet darauf hin, dass die ISR zu lange läuft oder blockiert ist.
*   **Git-Integration (DONE):** Projektänderungen (alle ESP32-Fixes, C3/C6 und XIAO-Targets) in einem neuen Feature-Branch (`feature/esp32-support`) committet und erfolgreich via SSH nach GitHub gepusst.
*   **PlatformIO Core Installation (DONE):** PlatformIO Core wurde erfolgreich auf dem System unter `/root/.platformio/penv/bin/pio` installiert und konfiguriert, um Uploads und Builds zu ermöglichen.
*   **ESP32 C Standard Library Conflicts (DONE):** `time_t` und `struct tm` Redefinitionen mit ESP-IDF System-Headern wurden behoben.
    *   `time_t` in `clib/ntp.h` wird nun nur bedingt definiert, um Konflikte zu vermeiden: `#ifndef ARM` wurde um `#if !defined(ESP32) && !defined(STM32)` erweitert.
    *   `struct tm` in `clib/ntp.h` wurde in `struct ntp_tm` umbenannt, um Konflikte mit dem Standard-`time.h` zu vermeiden. Entsprechende Anpassungen in `clib/ntp.c` und `clib/clock.c` wurden vorgenommen.
    *   Die `_BV` redefinition warning bleibt weiterhin bestehen, hat aber keine Auswirkungen auf die Funktionalität.
*   **Initialer Upload auf XIAO-ESP32-C3 (DONE):** Das neu erstellte Binary (v1.26.70 und Minimal-Sketches) wurde nun *mehrmals* erfolgreich auf den XIAO-ESP32-C3 geflasht.
*   **CLI Serial Test Script (`cul_exchange_test.py`, `cul_fifo_test.py`, `slowrf_test.py`) (DONE):** Python-basierte Skripte wurden entwickelt und erfolgreich zur Diagnose und Funktionsprüfung verwendet. `cul_test.py` wurde zu `cul_exchange_test.py` erweitert. `cul_fifo_test.py` und `slowrf_test.py` wurden neu erstellt.
*   **XIAO-ESP32-C3 Pin-Re-Assignment (DONE - RESOLVED Boot- & Serial-Output-Problem):**
    *   **Ursache (CONFIRMED):** Strapping-Pin-Konflikte (insbesondere GPIO 8, 9 und potenziell 2) verhinderte den Applikationsstart des XIAO-ESP32-C3, da diese Pins den Bootloader-Modus erzwingen können, wenn sie beim Reset auf einem kritischen Pegel sind.
    *   **Fix:** Das Pinout für den XIAO-ESP32-C3 wurde in der Hardware und in `platformio.ini` angepasst, um die Verwendung der kritischen Strapping-Pins für SPI und GDO zu vermeiden.
    *   **Angewandtes Pinout (Verifiziert & Funktional):**
        *   **SCK:** GPIO 6 (XIAO Pin D4), **MISO:** GPIO 10 (XIAO Pin D10), **MOSI:** GPIO 7 (XIAO Pin D5), **CS:** GPIO 5 (XIAO Pin D3)
        *   **GDO0:** GPIO 3 (Pin D1), **GDO2:** GPIO 4 (Pin D2)
        *   **LED:** GPIO 21 (XIAO Pin D6)
    *   **Verifizierung:** Die erfolgreiche Kommunikation mit dem CC1101 (Lesen von Registern wie `C00`, `C01`, `C02`, `C35`) und das korrekte Booten der Firmware bestätigen die Funktionalität des neuen Pinouts. Der `gdo_isr_count` debug counter zeigt die korrekte Interrupt-Aktivität an.
*   **`ttydata.h` Function Pointer Type Fix (DONE):** Der Typ des Funktionszeigers `fn` innerhalb der `t_fntab` Struktur in `culfw/clib/ttydata.h` wurde von `void (* const fn)(char *)` auf `void (* const fn)(uint8_t *)` geändert. Ein anschließender `invalid conversion` Compilerfehler in `main.cpp` wurde durch einen Cast behoben.
*   **`cc1100.c` Serial Output Conditional Compilation (DONE):** `Serial.println` Aufrufe in `culfw/clib/cc1100.c` wurden angepasst (z.B. durch `DBG()` ersetzt oder unter `#if defined(ESP32_DEBUG)` gestellt), um Konflikte mit der `Serial` Objektdefinition in der ESP32-Umgebung zu vermeiden und die Kompilierung erfolgreich zu machen.
*   **`eeprom_factory_reset` (command `e`) Fix (DONE):** Ein Absturz beim Ausführen des `e` Befehls wurde behoben, indem `esp_restart()` für ESP32-Targets korrekt in `fncollection.c` implementiert wurde.
*   **GDO Pin Mapping für Protokoll-spezifische Modi (DONE):**
    *   Die CC1101-Registerkonfigurationen (`MORITZ_CFG`, `ASKSIN_CFG`) in `clib/rf_moritz.c` und `clib/rf_asksin.c` wurden angepasst, um den "Packet Received"-Status über `IOCFG0` (GDO0, Register `0x02`) auszugeben, anstatt `IOCFG2` (GDO2, Register `0x00`), um die Konsistenz mit dem auf dem XIAO-ESP32-C3 verwendeten GDO0-Pin für Interrupts herzustellen.
    *   Die Standard-SlowRF-Konfiguration (`CC1100_CFG`) in `clib/cc1100.c` wurde auf `IOCFG0 = 0x0D` (Serial Data Output) umgestellt, um das Bit-Sampling in `rf_receive.c` zu ermöglichen.

## 4. Known Issues & Next Steps
*   **RF-Datenaustausch (Slow RF - FS20, Intertechno):**
    *   **Problem:** Der bidirektionale Datenaustausch von "Slow RF" Protokollen (FS20, Intertechno) zwischen dem XIAO-ESP32-C3 und einem Referenz-CUL funktioniert noch nicht zuverlässig.
    *   **Beobachtung:** Nach der Umstellung von `IOCFG0` auf `0x0D` (Serial Data Output) für SlowRF werden bei FS20-Übertragungen Interrupts auf GDO0 erkannt (hoher `gdo_isr_count`), und der ESP32 beginnt, empfangene Daten als FS20 zu interpretieren. Allerdings führt die Verarbeitung in der ISR (`CC1100_in_callback` / `RfAnalyze_Task`) zu einem **Interrupt Watchdog Timeout (Guru Meditation Error)** auf CPU0. Dies deutet darauf hin, dass die ISR zu lange läuft oder blockiert ist.
    *   **Status FIFO-basierte Protokolle (MAX!/Moritz):** Erfolgreich getestet. Bidirektionaler Datenaustausch für MAX!-Pakete funktioniert einwandfrei. Dies bestätigt die grundlegende Funktionsfähigkeit der SPI-Kommunikation und CC1101-Initialisierung für Paket-basierte Protokolle im FIFO-Modus.
*   **Kompilierungsfehler und Warnungen:**
    *   Die `_BV` redefinition warning bleibt weiterhin bestehen.
    *   Re-emergence of `sei()` and `cli()` redefinition warnings, specifically observed in `clib/intertechno.c`, leading to compilation failures. Dies deutet auf tiefergehende Konflikte zwischen AVR-spezifischen Headern (`avr/interrupt.h`, `avr/io.h`) und dem Arduino/ESP-IDF-Framework hin, die gelöst werden müssen.
*   **Weitere Entwicklung:**
    *   **Priorität 1: Behebung des Interrupt Watchdog Timeouts für SlowRF:** Analyse und Optimierung der GDO0-ISR und der `RfAnalyze_Task`-Routinen in `rf_receive.c`, um sicherzustellen, dass sie nicht die CPU blockieren und innerhalb der WDT-Grenzen arbeiten. Dies könnte die Optimierung der Puls-Dekodierung, die Verwendung von Fast-RF-Modi oder DMA für die Datenaufnahme umfassen.
    *   **Priorität 2: Behebung der Kompilierungsfehler/Warnungen:** Überarbeitung der betroffenen C-Dateien (insbesondere `clib/intertechno.c`) und deren Includes, um Konflikte mit dem ESP32-Framework zu vermeiden, oder Einführung spezifischer `#ifdef` Guards für ESP32.
    *   Überprüfung der CC1101-Konfigurationen (`MDMCFG` Register) für die Slow RF-Modi auf optimale Werte im Kontext der ESP32-Geschwindigkeit.
    *   Optimierung des SPI-Timings, falls erforderlich, um die Latenz weiter zu reduzieren.
    *   Implementierung eines Web-Interfaces für ESP32.