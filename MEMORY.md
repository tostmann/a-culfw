```markdown
# CULFW32 Project Status

## 1. General Project Information
*   **Project Name:** CULFW32 (Portierung der a-culfw auf 32-Bit-Mikrocontroller)
*   **Repository:** `tostmann/a-culfw.git` (forked)
*   **Local Codebase:** `/opt/ai_builder_data/users/763684262/projects/CULFW32/culfw/`
*   **Build System:** PlatformIO
*   **Target Architectures:** AVR, STM32, ESP32 (RISC-V)

## 2. Supported Boards & Firmware Versions
*   **Firmware Version:** 1.26.61 (latest build, automatisch inkrementiert)
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
*   **XIAO-ESP32-C3 Target (Builds successful with safe pinout. Hardware boot-problem with CC1101 still requires verification):**
    *   `XIAO-ESP32-C3-factory.bin` (Vollständiges Image für `0x0` - **Erfolgreich erstellt mit sicherem Pinout und korrektem Build-Pfad vom Projekt-Root**)
    *   `XIAO-ESP32-C3.bin` (Nur Applikation für OTA - **Erfolgreich erstellt mit sicherem Pinout und korrektem Build-Pfad vom Projekt-Root**)
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
*   **ESP32 USB-CDC & Konsole (DONE):**
    *   `ARDUINO_USB_MODE=1` und `ARDUINO_USB_CDC_ON_BOOT=1` für direkte USB-Serial über `Serial`-Objekt aktiviert.
    *   LED-Blinken (3x) beim Booten zur visuellen Rückmeldung.
    *   Nicht-blockierende I/O (`availableForWrite()`) und robuste Eingabeverarbeitung (`analyze_ttydata`) durch `Serial.flush()` und `delay(1)` in der Hauptschleife optimiert, um Eingaben zuverlässiger zu verarbeiten und Stabilität zu verbessern.
*   **ESP32 Pointer-Handling Fix (DONE):** `ttydata.c` korrigiert, um 32-Bit-Funktionspointer auf ESP32/ARM korrekt zu behandeln (Behebung des `Instruction access fault`-Absturzes, verursacht durch 16-Bit-AVR-Makro `__LPM_word` beim Auslesen der `fntab`).
*   **Flexible Pin-Konfiguration (DONE):** `board.h` (für ESP32) verwendet jetzt `#ifndef`-Guards, wodurch Pin-Definitionen über `build_flags` in `platformio.ini` für spezifische Board-Targets überschrieben werden können.
*   **PlatformIO Build-System-Fixes & Pfad-Standardisierung für ESP32-Targets (DONE):**
    *   Die globale `src_dir = .` Einstellung wurde in die `[platformio]` Sektion der Haupt-`platformio.ini` (`culfw/platformio.ini`) verschoben, um das Projekt-Root-Verzeichnis als Quellverzeichnis für alle Targets zu definieren.
    *   Alle `build_flags` für Include-Pfade (`-I`) und `build_src_filter` in `culfw/Devices/ESP32/platformio.ini` wurden für die ESP32-Targets (ESP32-C3, XIAO-ESP32-C3, ESP32-C6) auf korrekte relative Pfade zum Projekt-Root (`./`, `clib`, `ESP32` etc.) angepasst.
    *   Dies ermöglicht nun den erfolgreichen Build aller ESP32-Targets (`ESP32-C3`, `XIAO-ESP32-C3`, `ESP32-C6`) aus dem Projekt-Root (`culfw/`) mit dem Kommando `pio run -d culfw/ -e <env_name>`.
    *   Alle ESP32-Targets bauen nun erfolgreich und generieren sowohl `factory.bin` als auch `app.bin`.
*   **Git-Integration (DONE):** Projektänderungen (alle ESP32-Fixes, C3/C6 und XIAO-Targets) in einem neuen Feature-Branch (`feature/esp32-support`) committet und erfolgreich via SSH nach GitHub gepusht. Ein neuer SSH-Key wurde auf dem VPS generiert und dem Benutzer zur Registrierung bei GitHub bereitgestellt, um den Push zu ermöglichen.

## 4. Known Issues & Next Steps
*   **XIAO-ESP32-C3 Boot-Problem mit CC1101 (Hardware-Verifikation ausstehend):**
    *   **Problem:** Das XIAO-ESP32-C3-Board bootet nicht zuverlässig, wenn der CC1101 vollständig verkabelt ist (insbesondere mit angeschlossenen CS/GDO-Pins), was zu Abstürzen wie `Guru Meditation Error: Instruction access fault` führt.
    *   **Vermutete Ursache:** Interferenz des CC1101 mit den "Strapping Pins" des ESP32-C3 während des Bootvorgangs. Speziell GPIO 9 (MISO) ist ein kritischer Strapping-Pin, der beim Booten auf HIGH liegen muss. Wenn der CC1101 durch einen nicht initialisierten CS-Pin aktiviert wird, kann er die MISO-Leitung auf LOW ziehen und den ESP32 in einen falschen Boot-Modus zwingen. Auch GPIO 2 ist ein kritischer Strapping-Pin.
    *   **Status des Build-Systems (DONE):** Die `platformio.ini` wurde erfolgreich mit dem sicheren Pinout (CS auf GPIO5, LED auf GPIO21, etc.) und den korrigierten Build-Pfaden aktualisiert, sodass das `XIAO-ESP32-C3-factory.bin` Image nun erfolgreich aus dem Projekt-Root erstellt wird.
    *   **Nächste Schritte (Manuelle Hardware-Verifikation UNBEDINGT erforderlich):**
        1.  **Flashen des Images:** Flashe das erfolgreich erstellte `XIAO-ESP32-C3-factory.bin` (Version 1.26.61) auf das Board.
        2.  **Umverdrahtung der Hardware:** Schließe den CC1101 am XIAO-ESP32-C3 gemäß dem **oben definierten Pinout** an: SCK=GPIO8, MISO=GPIO9, MOSI=GPIO10, **CS=GPIO5**, GDO0=GPIO3, GDO2=GPIO4, LED=GPIO21.
        3.  **Test des Bootvorgangs:** Überprüfe, ob das Board nun stabil bootet.
    *   **Potenzieller weiterer Fix:** Sollte das Problem weiterhin bestehen, wird ein **10kΩ Pull-Up-Widerstand am CS-Pin (GPIO 5)** empfohlen, um den CC1101 während des Bootvorgangs sicher zu deaktivieren.
*   **CC1101 Funktionstests (Pending):** Sobald das XIAO-ESP32-C3 zuverlässig mit angeschlossener Hardware bootet und ein funktionierendes Image geflasht wurde, werden vollständige Funktionstests des CC1101 (Befehle `C` für Registerausgabe, `V` für Versions-/Frequenzprüfung, `X21` für FS20-Empfang) durchgeführt.
*   **Weitere Entwicklung:** Optimierung des SPI-Timings, Implementierung eines Web-Interfaces für ESP32.
```