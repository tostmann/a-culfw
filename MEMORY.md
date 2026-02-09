```markdown
# CULFW32 Project Status

## 1. General Project Information
*   **Project Name:** CULFW32 (Portierung der a-culfw auf 32-Bit-Mikrocontroller)
*   **Repository:** `tostmann/a-culfw.git` (forked)
*   **Local Codebase:** `/opt/ai_builder_data/users/763684262/projects/CULFW32/culfw/`
*   **Build System:** PlatformIO
*   **Target Architectures:** AVR, STM32, ESP32 (RISC-V)

## 2. Supported Boards & Firmware Versions
*   **Firmware Version:** 1.26.55 (latest build, automatisch inkrementiert - Die AI scheiterte wiederholt daran, das XIAO-Image mit korrigiertem Pinout aufgrund hartnäckiger PlatformIO-Build-Probleme zu erstellen. Die `platformio.ini` wurde auf den Ursprungszustand zurückgesetzt.)
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
*   **XIAO-ESP32-C3 Target (In Entwicklung - Boot-Problem & Build-Problem):**
    *   `XIAO-ESP32-C3-factory.bin` (Vollständiges Image für `0x0` - **aktuell nicht durch AI erstellbar, manuelle Korrektur der `platformio.ini` erforderlich**)
    *   `XIAO-ESP32-C3.bin` (Nur Applikation für OTA - **aktuell nicht durch AI erstellbar, manuelle Korrektur der `platformio.ini` erforderlich**)
    *   **Korrigiertes Pinout (zum manuellen Anwenden - ist in der untenstehenden `platformio.ini`-Konfiguration enthalten):** LED=GPIO21 (Pin D6), SPI: SCK=GPIO8 (Pin D8), MISO=GPIO9 (Pin D9), MOSI=GPIO10 (Pin D10), CS=GPIO5 (Pin D3), GDO0=GPIO3 (Pin D1), GDO2=GPIO4 (Pin D2).

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
    *   Nicht-blockierende I/O (`availableForWrite()`) und robuste Eingabeverarbeitung (`analyze_ttydata`) durch `Serial.flush()` und `delay(1)` in der Hauptschleife optimiert, um Eingaben zuverlässiger zu verarbeiten und Stabilität zu verbessern.
*   **ESP32 Pointer-Handling Fix (DONE):** `ttydata.c` korrigiert, um 32-Bit-Funktionspointer auf ESP32/ARM korrekt zu behandeln (Behebung des `Instruction access fault`-Absturzes, verursacht durch 16-Bit-AVR-Makro `__LPM_word` beim Auslesen der `fntab`).
*   **Flexible Pin-Konfiguration (DONE):** `board.h` (für ESP32) verwendet jetzt `#ifndef`-Guards, wodurch Pin-Definitionen über `build_flags` in `platformio.ini` für spezifische Board-Targets überschrieben werden können.
*   **Git-Integration (DONE):** Projektänderungen (alle ESP32-Fixes, C3/C6 und XIAO-Targets) in einem neuen Feature-Branch (`feature/esp32-support`) committet und erfolgreich via SSH nach GitHub gepusht. Ein neuer SSH-Key wurde auf dem VPS generiert und dem Benutzer zur Registrierung bei GitHub bereitgestellt, um den Push zu ermöglichen.

## 4. Known Issues & Next Steps
*   **XIAO-ESP32-C3 Boot-Problem mit CC1101 (Benötigt manuelle Intervention):**
    *   **Problem:** Das XIAO-ESP32-C3-Board bootet nicht zuverlässig, wenn der CC1101 vollständig verkabelt ist (insbesondere mit angeschlossenen CS/GDO-Pins), was zu Abstürzen wie `Guru Meditation Error: Instruction access fault` führt.
    *   **Vermutete Ursache:** Interferenz des CC1101 mit den "Strapping Pins" des ESP32-C3 während des Bootvorgangs. Speziell GPIO 9 (MISO) ist ein kritischer Strapping-Pin, der beim Booten auf HIGH liegen muss. Wenn der CC1101 durch einen nicht initialisierten CS-Pin aktiviert wird, kann er die MISO-Leitung auf LOW ziehen und den ESP32 in einen falschen Boot-Modus zwingen. Auch GPIO 2 ist ein kritischer Strapping-Pin.
    *   **Status des Build-Systems:** Die AI scheiterte wiederholt daran, die Pin-Konfiguration und die Build-Pfade (extra_scripts, include-Pfade, src_dir, build_src_filter) in der `platformio.ini` automatisch zu aktualisieren und das Image zu bauen. Trotz zahlreicher Korrekturversuche traten hartnäckige Probleme mit der Auflösung relativer Pfade und dem Auffinden von Quelldateien durch PlatformIO in dieser komplexen Projektstruktur auf. Alle automatisierten Korrekturversuche schlugen fehl, und die `platformio.ini` wurde daher auf den ursprünglichen Zustand zurückgesetzt. Dies bedeutet, dass das XIAO-ESP32-C3 Target derzeit in der `platformio.ini` noch das inkorrekte, nicht-sichere Pinout verwendet und manuell korrigiert werden muss, bevor ein erfolgreicher Build möglich ist.
    *   **Nächste Schritte (Manuelle Korrektur UNBEDINGT erforderlich):**
        1.  **Manuelles Update der `platformio.ini`:** Ersetze den kompletten `[env:XIAO-ESP32-C3]`-Block in der Datei `/opt/ai_builder_data/users/763684262/projects/CULFW32/culfw/Devices/ESP32/platformio.ini` mit dem folgenden korrigierten Codeblock (dieser enthält das sichere Pinout und die angepassten Build-Pfade):

        ```ini
        [env:XIAO-ESP32-C3]
        platform = espressif32
        board = seeed_xiao_esp32c3
        framework = arduino
        monitor_speed = 115200

        ; Flash settings for better compatibility
        board_build.flash_mode = dio
        board_build.f_flash = 40000000L

        extra_scripts =
            pre:../../scripts/increment_version.py
            post:../../scripts/merge_bin.py
            post:../../scripts/collect_binaries.py

        build_flags =
            -DESP32_C3
            -DUSE_HAL
            -DARDUINO_USB_MODE=1
            -DARDUINO_USB_CDC_ON_BOOT=1
            -DHAS_CC1100
            -DBOARD_NAME=\"XIAO-ESP32-C3\"
            -DBOARD_ID_STR=\"XIAO-ESP32-C3\"
            ; --- Sicheres Pinout ---
            -DSPI_SCLK=8
            -DSPI_MISO=9
            -DSPI_MOSI=10
            -DSPI_SS=5
            -DCC1100_CS_PIN=5
            -DGDO0_PIN=3
            -DGDO2_PIN=4
            -DLED_PIN=21
            ; --- Korrigierte Include-Pfade ---
            -I .
            -I ../../
            -I ../../clib
            -I ../../clib/mbus

        build_src_filter =
            +<main.cpp>
            +<../../ESP32/hal.cpp>
            +<../../clib/cc1100.c>
            +<../../clib/cc1101_pllcheck.c>
            +<../../clib/clock.c>
            +<../../clib/display.c>
            +<../../clib/fastrf.c>
            +<../../clib/fband.c>
            +<../../clib/fht.c>
            +<../../clib/fncollection.c>
            +<../../clib/hw_autodetect.c>
            +<../../clib/intertechno.c>
            +<../../clib/kopp-fc.c>
            +<../../clib/multi_CC.c>
            +<../../clib/rf_asksin.c>
            +<../../clib/rf_maico.c>
            +<../../clib/rf_mbus.c>
            +<../../clib/mbus/*.c>
            +<../../clib/rf_mode.c>
            +<../../clib/rf_moritz.c>
            +<../../clib/rf_native.c>
            +<../../clib/lacrosse.c>
            +<../../clib/rf_router.c>
            +<../../clib/rf_receive_bucket.c>
            +<../../clib/rf_receive_esa.c>
            +<../../clib/rf_receive_hms.c>
            +<../../clib/rf_receive_it.c>
            +<../../clib/rf_receive_revolt.c>
            +<../../clib/rf_receive_tcm97001.c>
            +<../../clib/rf_receive_tx3.c>
            +<../../clib/rf_receive.c>
            +<../../clib/rf_rwe.c>
            +<../../clib/rf_send.c>
            +<../../clib/rf_zwave.c>
            +<../../clib/ringbuffer.c>
            +<../../clib/somfy_rts.c>
            +<../../clib/stringfunc.c>
            +<../../clib/ttydata.c>
            +<../../clib/onewire.c>
        ```
        2.  **Manuelles Builden:** Starte den Build für das XIAO-ESP32-C3 Target lokal (z.B. mit `platformio run -e XIAO-ESP32-C3` im Verzeichnis `/opt/ai_builder_data/users/763684262/projects/CULFW32/culfw/Devices/ESP32`).
        3.  **Umverdrahtung der Hardware:** Schließe den CC1101 am XIAO-ESP32-C3 gemäß dem **oben definierten Pinout** an: SCK=GPIO8, MISO=GPIO9, MOSI=GPIO10, **CS=GPIO5**, GDO0=GPIO3, GDO2=GPIO4, LED=GPIO21.
        4.  **Flashen des Images:** Flashe das manuell erstellte `XIAO-ESP32-C3-factory.bin` auf das Board.
        5.  **Test des Bootvorgangs:** Überprüfe, ob das Board nun stabil bootet.
    *   **Potenzieller weiterer Fix:** Sollte das Problem weiterhin bestehen, wird ein **10kΩ Pull-Up-Widerstand am CS-Pin (GPIO 5)** empfohlen, um den CC1101 während des Bootvorgangs sicher zu deaktivieren.
*   **CC1101 Funktionstests (Pending):** Sobald das XIAO-ESP32-C3 zuverlässig mit angeschlossener Hardware bootet und ein funktionierendes Image geflasht wurde, werden vollständige Funktionstests des CC1101 (Befehle `C` für Registerausgabe, `V` für Versions-/Frequenzprüfung, `X21` für FS20-Empfang) durchgeführt.
*   **Weitere Entwicklung:** Optimierung des SPI-Timings, Implementierung eines Web-Interfaces für ESP32.
```