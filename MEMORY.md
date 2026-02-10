```markdown
# CULFW32 Project Status

## 1. General Project Information
*   **Project Name:** CULFW32 (Portierung der a-culfw auf 32-Bit-Mikrocontroller)
*   **Repository:** `tostmann/a-culfw.git` (forked)
*   **Local Codebase:** `/opt/ai_builder_data/users/763684262/projects/CULFW32/culfw/`
*   **Build System:** PlatformIO
*   **Target Architectures:** AVR, STM32, ESP32 (RISC-V)

## 2. Supported Boards & Firmware Versions
*   **Firmware Version:** 1.26.70 (latest build, automatisch inkrementiert nach Fixes)
*   **AVR Targets (Verified):**
    *   `CUL_V3.hex`
    *   `nanoCUL868.hex`
    *   `nanoCUL433.hex`
*   **STM32 Targets (Verified & Fixed):**
    *   `MapleCUNx4_W5100_BL.bin`
    *   `MapleCUNx4_W5500_BL.bin`
*   **ESP32 Targets (Stable & Functional - *except current XIAO-ESP32-C3 boot issue*):**
    *   **ESP32-C3:**
        *   `ESP32-C3-factory.bin` (Vollständiges Image für `0x0`)
        *   `ESP32-C3.bin` (Nur Applikation für OTA)
        *   **Pinout:** LED=GPIO4, SPI: SCK=6, MISO=5, MOSI=7, CS=10, GDO0=2, GDO2=3.
    *   **ESP32-C6:**
        *   `ESP32-C6-factory.bin` (Vollständiges Image für `0x0`)
        *   `ESP32-C6.bin` (Nur Applikation für OTA)
        *   **Pinout:** LED=GPIO8 (DevKitC-1 Standard), SPI Pins wie in `board.h` definiert.
    *   **XIAO-ESP32-C3:** (Builds successfully, but boot/serial issues prevent full verification)
        *   `XIAO-ESP32-C3-factory.bin` (Vollständiges Image für `0x0`)
        *   `XIAO-ESP32-C3.bin` (Nur Applikation für OTA)
        *   **Angewendetes Pinout (Verifiziert im Build):** LED=GPIO21 (Pin D6), SPI: SCK=GPIO8 (Pin D8), MISO=GPIO9 (Pin D9), MOSI=GPIO10 (Pin D10), CS=GPIO5 (Pin D3), GDO0=GPIO3 (Pin D1), GDO2=GPIO4 (Pin D2).

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
*   **ESP32 USB-CDC & Konsole (DONE - Initiales Setup, ABER Boot-Probleme bestehen):**
    *   `ARDUINO_USB_MODE=1` und `ARDUINO_USB_CDC_ON_BOOT=1` für direkte USB-Serial über `Serial`-Objekt aktiviert.
    *   Doppelte Initialisierungen im `setup()` wurden entfernt.
    *   `Serial` wird konditional zu `USBSerial` definiert, um sicherzustellen, dass die USB-CDC-Schnittstelle verwendet wird.
    *   `Serial.setTxTimeoutMs(0)` wurde für nicht-blockierende serielle Ausgaben hinzugefügt.
*   **ESP32 Pointer-Handling Fix (DONE):** `ttydata.c` korrigiert, um 32-Bit-Funktionspointer auf ESP32/ARM korrekt zu behandeln (Behebung des `Instruction access fault`-Absturzes, verursacht durch 16-Bit-AVR-Makro `__LPM_word` beim Auslesen der `fntab`).
*   **Flexible Pin-Konfiguration (DONE):** `board.h` (für ESP32) verwendet jetzt `#ifndef`-Guards, wodurch Pin-Definitionen über `build_flags` in `platformio.ini` für spezifische Board-Targets überschrieben werden können.
*   **PlatformIO Build-System-Fixes & Pfad-Standardisierung für ESP32-Targets (DONE):**
    *   Die globale `src_dir = .` Einstellung wurde in die `[platformio]` Sektion der Haupt-`platformio.ini` (`culfw/platformio.ini`) verschoben, um das Projekt-Root-Verzeichnis als Quellverzeichnis für alle Targets zu definieren.
    *   Alle `build_flags` für Include-Pfade (`-I`) und `build_src_filter` in `culfw/Devices/ESP32/platformio.ini` wurden für die ESP32-Targets (ESP32-C3, XIAO-ESP32-C3, ESP32-C6) auf korrekte relative Pfade zum Projekt-Root (`./`, `clib`, `ESP32` etc.) angepasst.
    *   Dies ermöglicht nun den erfolgreichen Build aller ESP32-Targets (`ESP32-C3`, `XIAO-ESP32-C3`, `ESP32-C6`) aus dem Projekt-Root (`culfw/`) mit dem Kommando `pio run -d culfw/ -e <env_name>`.
    *   Alle ESP32-Targets bauen nun erfolgreich und generieren sowohl `factory.bin` als auch `app.bin`.
*   **AVR-spezifische Header-Dateien für ESP32 (DONE):** Die `#include <avr/io.h>` und `#include <avr/pgmspace.h>` Pfade wurden in den `build_flags` für ESP32-Targets korrekt referenziert, um Kompilierungsfehler zu beheben (Warnung `_BV` Redefinition bleibt).
*   **GDO-Interrupt-Handling & Debugging für ESP32 (DONE - Code-Implementierung, ABER Boot-Probleme blockieren Verifikation):**
    *   **Problemursache (ehemals):** Der GDO0-Interrupt wurde in `hal_CC_GDO_init()` zu früh aktiviert, was zu einem "Interrupt Storm" führte.
    *   **Fix (Code):** Der Interrupt wird nun explizit nach vollständiger CC1101-Initialisierung in `setup()` aktiviert. `pinMode(GDO0_PIN, INPUT_PULLUP)` und `IRAM_ATTR` für den ISR-Handler wurden gesetzt. Der `gdo_isr_count` zur Überwachung der Interrupt-Aktivität wurde implementiert.
    *   Diese Maßnahmen haben die *ursprüngliche* Systemblockade behoben. Die eigentliche Paketerkennung kann jedoch aufgrund des aktuellen Boot-Problems noch nicht verifiziert werden.
*   **Git-Integration (DONE):** Projektänderungen (alle ESP32-Fixes, C3/C6 und XIAO-Targets) in einem neuen Feature-Branch (`feature/esp32-support`) committet und erfolgreich via SSH nach GitHub gepusst.
*   **PlatformIO Core Installation (DONE):** PlatformIO Core wurde erfolgreich auf dem System unter `/root/.platformio/penv/bin/pio` installiert und konfiguriert, um Uploads und Builds zu ermöglichen.
*   **ESP32 C Standard Library Conflicts (DONE):** `time_t` und `struct tm` Redefinitionen mit ESP-IDF System-Headern wurden behoben.
    *   `time_t` in `clib/ntp.h` wird nun nur bedingt definiert, um Konflikte zu vermeiden: `#ifndef ARM` wurde um `#if !defined(ESP32) && !defined(STM32)` erweitert.
    *   `struct tm` in `clib/ntp.h` wurde in `struct ntp_tm` umbenannt, um Konflikte mit dem Standard-`time.h` zu vermeiden. Entsprechende Anpassungen in `clib/ntp.c` und `clib/clock.c` wurden vorgenommen.
    *   Die `_BV` redefinition warning bleibt weiterhin bestehen, hat aber keine Auswirkungen auf die Funktionalität.
*   **Initialer Upload auf XIAO-ESP32-C3 (DONE):** Das neu erstellte Binary (v1.26.70) wurde nun *mehrmals* erfolgreich auf den XIAO-ESP32-C3 geflasht.
*   **CLI Serial Test Script (DONE):** Ein Python-basiertes Skript (`cul_test.py`) wurde entwickelt, um serielle Kommunikation zu testen. Es wurde erfolgreich mit einem Legacy CUL868 (`/dev/ttyACM1`) verifiziert.

## 4. Known Issues & Next Steps
*   **XIAO-ESP32-C3 Boot- & Serial-Output-Problem (CRITICAL - In Progress):**
    *   **Problem:** Nach dem Flashen der Firmware (sowohl einer vollständigen CULFW-Version als auch eines minimalistischen "Hello World"-Sketches) bootet der XIAO-ESP32-C3 nicht in die Applikation. Stattdessen verbleibt er im "wait usb download"-Modus, und es wird keine serielle Ausgabe empfangen, weder im `pio device monitor` noch mit direkten `pyserial`-Skripten.
    *   **Identifizierte Hauptursache:** Es besteht der dringende Verdacht eines **Strapping-Pin-Konflikts**. GPIO 9 (MISO-Pin im aktuellen Pinout des XIAO-ESP32-C3) ist ein Strapping-Pin. Wenn dieser beim Reset auf "Low" gezogen wird (was durch den angeschlossenen CC1101 oder fehlende Pull-ups geschehen kann), erzwingt dies den Bootloader-Modus und verhindert den Start der Applikation. Andere Strapping-Pins (GPIO 8/SCK) könnten ebenfalls eine Rolle spielen.
    *   **Aktueller Status (Builds & Flashes SUCCESSFUL, Boot-Modus PROBLEMATISCH, Serielle Ausgabe FEHLT):**
        1.  Die Firmware (sowohl CULFW als auch ein Minimal-Sketch) lässt sich erfolgreich auf den XIAO-ESP32-C3 flashen, auch mit `factory.bin` auf `0x0`.
        2.  `esptool.py` kann mit dem Chip kommunizieren und Flash-Informationen auslesen, jedoch startet der Chip nach einem Reset durch `esptool.py` nicht die Applikation, sondern verbleibt im "wait usb download"-Modus.
        3.  Alle Versuche, serielle Ausgaben zu erhalten (inkl. expliziter `USBSerial`-Nutzung, `setTxTimeoutMs(0)`, manuellen DTR/RTS-Resets per Skript), schlugen fehl.
        4.  Der `cul_test.py` Skript bestätigt das Ausbleiben jeglicher serieller Antwort vom XIAO.
        5.  **Obsolet / Geklärt:** Die ursprünglichen Probleme mit Port-Locking (`[Errno 11] Resource temporarily unavailable`) und `termios.error` beim `pio device monitor` wurden durch direkte Ausführung des Monitors oder die Verwendung des `cul_test.py` Skripts umgangen und sind nicht mehr die primäre Blockade.

*   **Nächste Schritte zur Behebung des Boot- & Serial-Output-Problems:**
    1.  **Pin-Re-Assignment:** Das Pinout für den XIAO-ESP32-C3 muss überprüft und angepasst werden, um die Verwendung von kritischen Strapping-Pins (insbesondere GPIO 9/MISO und GPIO 8/SCK) für den SPI-Bus zu vermeiden. Neue, unkritische Pins sollten für den CC1101 zugewiesen werden.
    2.  **Hardware-Verifikation/Modifikation:** Falls eine Pin-Re-Assignment nicht sofort möglich ist, muss geprüft werden, ob Pull-Up-Widerstände an den Strapping-Pins GPIO 9 und GPIO 8 (im Idealfall 10kΩ) während des Bootvorgangs sicherstellen, dass diese auf "High" gehalten werden, um den Applikationsstart zu ermöglichen.
    3.  **Minimal-Sketch mit neuem Pinout:** Nach der Anpassung der Pins (im Code und ggf. Hardware) sollte zuerst der minimalistische "Hello World" Sketch mit der LED-Blinkfunktion und `USBSerial.println()` erneut geflasht werden, um die serielle Ausgabe und den korrekten Boot der Applikation zu verifizieren.
    4.  **CULFW-Logik wiederherstellen:** Erst wenn der Minimal-Sketch stabil bootet und serielle Ausgaben liefert, sollte die vollständige CULFW-Logik mit den korrigierten Pins getestet werden.

*   **Weitere Entwicklung (aktuell pausiert):**
    *   Optimierung des SPI-Timings.
    *   Implementierung eines Web-Interfaces für ESP32.
```