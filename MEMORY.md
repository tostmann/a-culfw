```markdown
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
    *   **XIAO-ESP32-C3:** (Stable & Functional)
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
*   **GDO-Interrupt-Handling & Debugging für ESP32 (DONE - Code-Implementierung, VERIFIED nach Boot-Fix):**
    *   **Problemursache (ehemals):** Der GDO0-Interrupt wurde in `hal_CC_GDO_init()` zu früh aktiviert, was zu einem "Interrupt Storm" führte.
    *   **Fix (Code):** Der Interrupt wird nun explizit nach vollständiger CC1101-Initialisierung in `setup()` aktiviert. `pinMode(GDO0_PIN, INPUT_PULLUP)` und `IRAM_ATTR` für den ISR-Handler wurden gesetzt. Der `gdo_isr_count` zur Überwachung der Interrupt-Aktivität wurde implementiert.
    *   Diese Maßnahmen haben die *ursprüngliche* Systemblockade behoben und die Funktionalität kann nun nach dem gelösten Boot-Problem verifiziert werden.
*   **Git-Integration (DONE):** Projektänderungen (alle ESP32-Fixes, C3/C6 und XIAO-Targets) in einem neuen Feature-Branch (`feature/esp32-support`) committet und erfolgreich via SSH nach GitHub gepusst.
*   **PlatformIO Core Installation (DONE):** PlatformIO Core wurde erfolgreich auf dem System unter `/root/.platformio/penv/bin/pio` installiert und konfiguriert, um Uploads und Builds zu ermöglichen.
*   **ESP32 C Standard Library Conflicts (DONE):** `time_t` und `struct tm` Redefinitionen mit ESP-IDF System-Headern wurden behoben.
    *   `time_t` in `clib/ntp.h` wird nun nur bedingt definiert, um Konflikte zu vermeiden: `#ifndef ARM` wurde um `#if !defined(ESP32) && !defined(STM32)` erweitert.
    *   `struct tm` in `clib/ntp.h` wurde in `struct ntp_tm` umbenannt, um Konflikte mit dem Standard-`time.h` zu vermeiden. Entsprechende Anpassungen in `clib/ntp.c` und `clib/clock.c` wurden vorgenommen.
    *   Die `_BV` redefinition warning bleibt weiterhin bestehen, hat aber keine Auswirkungen auf die Funktionalität.
*   **Initialer Upload auf XIAO-ESP32-C3 (DONE):** Das neu erstellte Binary (v1.26.70 und Minimal-Sketches) wurde nun *mehrmals* erfolgreich auf den XIAO-ESP32-C3 geflasht.
*   **CLI Serial Test Script (`cul_test.py`) (DONE):** Ein Python-basiertes Skript (`cul_test.py`) wurde entwickelt, um serielle Kommunikation zu testen. Es wurde erfolgreich mit einem Legacy CUL868 (`/dev/ttyACM1`) verifiziert und zur Diagnose und Funktionsprüfung des XIAO-ESP32-C3 verwendet.
*   **XIAO-ESP32-C3 Pin-Re-Assignment (DONE):**
    *   **Ursache (CONFIRMED):** Strapping-Pin-Konflikte (insbesondere GPIO 8, 9 und potenziell 2) verhinderte den Applikationsstart des XIAO-ESP32-C3, da diese Pins den Bootloader-Modus erzwingen können, wenn sie beim Reset auf einem kritischen Pegel sind.
    *   **Fix:** Das Pinout für den XIAO-ESP32-C3 wurde in der Hardware und in `platformio.ini` angepasst, um die Verwendung der kritischen Strapping-Pins für SPI und GDO zu vermeiden.
    *   **Angewandtes Pinout (Verifiziert & Funktional):**
        *   **SCK:** GPIO 6 (XIAO Pin D4)
        *   **MISO:** GPIO 10 (XIAO Pin D10)
        *   **MOSI:** GPIO 7 (XIAO Pin D5)
        *   **CS:** GPIO 5 (XIAO Pin D3)
        *   **GDO0:** GPIO 3 (XIAO Pin D1)
        *   **GDO2:** GPIO 4 (XIAO Pin D2)
        *   **LED:** GPIO 21 (XIAO Pin D6)
    *   **Verifizierung:** Die erfolgreiche Kommunikation mit dem CC1101 (Lesen von Registern wie `C00`, `C01`, `C02`) und das korrekte Booten der Firmware bestätigen die Funktionalität des neuen Pinouts.

## 4. Known Issues & Next Steps
*   **XIAO-ESP32-C3 Boot- & Serial-Output-Problem (RESOLVED):**
    *   **Problem:** Nach dem Flashen der Firmware bootete der XIAO-ESP32-C3 nicht in die Applikation, verblieb im "wait usb download"-Modus, und es wurde keine serielle Ausgabe empfangen.
    *   **Lösung:** Die Ursache war ein Strapping-Pin-Konflikt, bei dem kritische Pins (insbesondere GPIO 8, 9) für den SPI-Bus genutzt wurden. Durch eine Hardware-Anpassung des Pinouts (siehe Abschnitt 3, "XIAO-ESP32-C3 Pin-Re-Assignment") wurden diese Pins vermieden. Die Firmware bootet nun zuverlässig, und die serielle Kommunikation funktioniert einwandfrei. Die CC1101-Kommunikation wurde ebenfalls erfolgreich verifiziert.

*   **Weitere Entwicklung:**
    *   Optimierung des SPI-Timings.
    *   Implementierung eines Web-Interfaces für ESP32.
```