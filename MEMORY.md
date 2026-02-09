# CULFW32 Project Status

## 1. General Project Information
*   **Project Name:** CULFW32 (Portierung der a-culfw auf 32-Bit-Mikrocontroller)
*   **Repository:** `tostmann/a-culfw.git` (forked)
*   **Local Codebase:** `/opt/ai_builder_data/users/763684262/projects/CULFW32/culfw/`
*   **Build System:** PlatformIO
*   **Target Architectures:** AVR, STM32, ESP32 (RISC-V)

## 2. Supported Boards & Firmware Versions
*   **Firmware Version:** 1.26.55 (latest build, automatisch inkrementiert)
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
*   **XIAO-ESP32-C3 Target (In Entwicklung - Boot-Problem):**
    *   `XIAO-ESP32-C3-factory.bin` (Vollständiges Image für `0x0`)
    *   `XIAO-ESP32-C3.bin` (Nur Applikation für OTA)
    *   **Aktuelles Pinout:** LED=GPIO21 (Pin D6), SPI: SCK=GPIO8 (Pin D8), MISO=GPIO9 (Pin D9), MOSI=GPIO10 (Pin D10), CS=GPIO5 (Pin D3), GDO0=GPIO3 (Pin D1), GDO2=GPIO4 (Pin D2).

## 3. Key Architectural Decisions & Changes
*   **Build-Prozess (DONE):** Alle ESP32-Targets generieren jetzt automatisch sowohl `factory.bin` (komplettes Image mit Bootloader/Partitionstabelle) als auch `app.bin` (für OTA).
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
    *   Nicht-blockierende I/O (`availableForWrite()`) und robuste Eingabeverarbeitung (`analyze_ttydata`).
*   **ESP32 Pointer-Handling Fix (DONE):** `ttydata.c` korrigiert, um 32-Bit-Funktionspointer auf ESP32/ARM korrekt zu behandeln, behebt `Instruction access fault`-Abstürze (verursacht durch 16-Bit-AVR-Makro).
*   **Flexible Pin-Konfiguration (DONE):** `board.h` (für ESP32) verwendet jetzt `#ifndef`-Guards, wodurch Pin-Definitionen über `build_flags` in `platformio.ini` für spezifische Board-Targets überschrieben werden können.
*   **Git-Integration (DONE):** Projektänderungen in einem neuen Feature-Branch (`feature/esp32-support`) committet und erfolgreich via SSH nach GitHub gepusht.

## 4. Known Issues & Next Steps
*   **XIAO-ESP32-C3 Boot-Problem mit CC1101 (Aktive Untersuchung):**
    *   **Problem:** Das XIAO-ESP32-C3-Board bootet nicht, wenn der CC1101 vollständig verkabelt ist, insbesondere wenn CS- und GDO-Pins angeschlossen sind, auch wenn GPIO2 frei ist.
    *   **Vermutete Ursache:** Interferenz des CC1101 mit den "Strapping Pins" des ESP32-C3 (GPIO 9/MISO und möglicherweise GPIO 8/SCK) während des Bootvorgangs. Wenn die CS-Leitung des CC1101 während des ESP32-Boots nicht zuverlässig auf HIGH (deaktiviert) gehalten wird, kann dies die MISO-Leitung (GPIO 9) auf LOW ziehen und den ESP32 in einen falschen Boot-Modus zwingen.
    *   **Aktueller Lösungsversuch:** CS wurde auf **GPIO 5 (Pin D3)** verschoben, während SCK, MISO, MOSI auf den Standard-SPI-Pins 8, 9, 10 verbleiben. GDO-Pins sind auf GPIO 3, 4.
    *   **Nächster Schritt:** Benutzer muss den CC1101 gemäß dem neuen Pinout am XIAO-ESP32-C3 umverdrahten und das neueste `XIAO-ESP32-C3-factory.bin` flashen.
    *   **Potenzieller weiterer Fix:** Sollte das Problem weiterhin bestehen, wird ein 10kΩ Pull-Up-Widerstand am CS-Pin (GPIO 5) oder das vorübergehende Trennen von MISO (GPIO 9) während des Bootvorgangs getestet, um die Strapping-Pin-Theorie zu bestätigen.
*   **CC1101 Funktionstests:** Sobald das XIAO-ESP32-C3 zuverlässig mit angeschlossener Hardware bootet, werden vollständige Funktionstests des CC1101 (Befehle `C`, `V`, `X21` für FS20-Empfang) durchgeführt.
*   **Weitere Entwicklung:** Optimierung des SPI-Timings, Implementierung eines Web-Interfaces für ESP32.