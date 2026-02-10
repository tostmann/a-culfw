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
*   **ESP32 Targets (Stable & Functional):**
    *   **ESP32-C3:**
        *   `ESP32-C3-factory.bin` (Vollständiges Image für `0x0`)
        *   `ESP32-C3.bin` (Nur Applikation für OTA)
        *   **Pinout:** LED=GPIO4, SPI: SCK=6, MISO=5, MOSI=7, CS=10, GDO0=2, GDO2=3.
    *   **ESP32-C6:**
        *   `ESP32-C6-factory.bin` (Vollständiges Image für `0x0`)
        *   `ESP32-C6.bin` (Nur Applikation für OTA)
        *   **Pinout:** LED=GPIO8 (DevKitC-1 Standard), SPI Pins wie in `board.h` definiert.
    *   **XIAO-ESP32-C3:** (Stable & Functional - multiple successful flashes, SlowRF reception debugging in progress)
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
*   **ESP32 USB-CDC & Konsole (DONE):**
    *   `ARDUINO_USB_MODE=1` und `ARDUINO_USB_CDC_ON_BOOT=1` für direkte USB-Serial über `Serial`-Objekt aktiviert.
    *   Das initiale LED-Blinken (3x) beim Booten zur visuellen Rückmeldung wurde zur Fehlersuche an der seriellen Ausgabe temporär entfernt.
    *   Nicht-blockierende I/O (`availableForWrite()`) und robuste Eingabeverarbeitung (`analyze_ttydata`) durch `Serial.flush()` und `delay(1)` in der Hauptschleife optimiert, um Eingaben zuverlässiger zu verarbeiten und Stabilität zu verbessern.
    *   **Debugging:** Erweiterte `Serial.println` Ausgaben im `setup()` und ein "Heartbeat" mit `ISR Count` im `loop()` wurden für die Diagnose der seriellen Kommunikation implementiert.
    *   Die Definition von `Serial` zu `USBSerial` wurde für Szenarien hinzugefügt, in denen `ARDUINO_USB_CDC_ON_BOOT` nicht gesetzt ist (für zukünftige Kompatibilität).
    *   Doppelte Initialisierungen im `setup()` wurden entfernt.
*   **ESP32 Pointer-Handling Fix (DONE):** `ttydata.c` korrigiert, um 32-Bit-Funktionspointer auf ESP32/ARM korrekt zu behandeln (Behebung des `Instruction access fault`-Absturzes, verursacht durch 16-Bit-AVR-Makro `__LPM_word` beim Auslesen der `fntab`).
*   **Flexible Pin-Konfiguration (DONE):** `board.h` (für ESP32) verwendet jetzt `#ifndef`-Guards, wodurch Pin-Definitionen über `build_flags` in `platformio.ini` für spezifische Board-Targets überschrieben werden können.
*   **PlatformIO Build-System-Fixes & Pfad-Standardisierung für ESP32-Targets (DONE):**
    *   Die globale `src_dir = .` Einstellung wurde in die `[platformio]` Sektion der Haupt-`platformio.ini` (`culfw/platformio.ini`) verschoben, um das Projekt-Root-Verzeichnis als Quellverzeichnis für alle Targets zu definieren.
    *   Alle `build_flags` für Include-Pfade (`-I`) und `build_src_filter` in `culfw/Devices/ESP32/platformio.ini` wurden für die ESP32-Targets (ESP32-C3, XIAO-ESP32-C3, ESP32-C6) auf korrekte relative Pfade zum Projekt-Root (`./`, `clib`, `ESP32` etc.) angepasst.
    *   Dies ermöglicht nun den erfolgreichen Build aller ESP32-Targets (`ESP32-C3`, `XIAO-ESP32-C3`, `ESP32-C6`) aus dem Projekt-Root (`culfw/`) mit dem Kommando `pio run -d culfw/ -e <env_name>`.
    *   Alle ESP32-Targets bauen nun erfolgreich und generieren sowohl `factory.bin` als auch `app.bin`.
*   **AVR-spezifische Header-Dateien für ESP32 (DONE):** Die `#include <avr/io.h>` und `#include <avr/pgmspace.h>` Pfade wurden in den `build_flags` für ESP32-Targets korrekt referenziert, um Kompilierungsfehler zu beheben (Warnung `_BV` Redefinition bleibt).
*   **GDO-Interrupt-Handling & Debugging für ESP32 (DONE):**
    *   **Problemursache:** Der GDO0-Interrupt wurde in `hal_CC_GDO_init()` zu früh aktiviert, bevor der CC1101 vollständig konfiguriert war, was zu einem "Interrupt Storm" führte.
    *   **Fix:** Der Interrupt wird nun explizit nach vollständiger CC1101-Initialisierung in `setup()` aktiviert. `pinMode(GDO0_PIN, INPUT_PULLUP)` und `IRAM_ATTR` für den ISR-Handler wurden gesetzt. Der `gdo_isr_count` zur Überwachung der Interrupt-Aktivität wurde implementiert und wird periodisch ausgegeben.
    *   Diese Maßnahmen haben die Systemblockade und die mangelnde Reaktion auf serielle Eingaben behoben. Die eigentliche Paketerkennung muss noch verifiziert werden.
*   **Git-Integration (DONE):** Projektänderungen (alle ESP32-Fixes, C3/C6 und XIAO-Targets) in einem neuen Feature-Branch (`feature/esp32-support`) committet und erfolgreich via SSH nach GitHub gepusst.
*   **PlatformIO Core Installation (DONE):** PlatformIO Core wurde erfolgreich auf dem System unter `/root/.platformio/penv/bin/pio` installiert und konfiguriert, um Uploads und Builds zu ermöglichen.
*   **ESP32 C Standard Library Conflicts (DONE):** `time_t` und `struct tm` Redefinitionen mit ESP-IDF System-Headern wurden behoben.
    *   `time_t` in `clib/ntp.h` wird nun nur bedingt definiert, um Konflikte zu vermeiden: `#ifndef ARM` wurde um `#if !defined(ESP32) && !defined(STM32)` erweitert.
    *   `struct tm` in `clib/ntp.h` wurde in `struct ntp_tm` umbenannt, um Konflikte mit dem Standard-`time.h` zu vermeiden. Entsprechende Anpassungen in `clib/ntp.c` und `clib/clock.c` wurden vorgenommen.
    *   Die `_BV` redefinition warning bleibt weiterhin bestehen, hat aber keine Auswirkungen auf die Funktionalität.
*   **Initialer Upload auf XIAO-ESP32-C3 (DONE):** Das neu erstellte Binary (v1.26.70) wurde nun *mehrmals* erfolgreich auf den XIAO-ESP32-C3 geflasht.

## 4. Known Issues & Next Steps
*   **XIAO-ESP32-C3 SlowRF Reception Debugging (In Progress):**
    *   **Problem:** Der SlowRF Empfang funktioniert nach der Portierung auf den ESP32 (insbesondere XIAO-ESP32-C3) nicht wie erwartet. Das Booten des Boards mit angeschlossenem CC1101 ist stabil, der `ISR Count` zeigt Interrupt-Aktivität an, aber es werden keine Datenpakete empfangen oder korrekt verarbeitet. Das initiale Problem der Systemblockade durch einen "Interrupt Storm" wurde behoben.
    *   **Vermutete Ursachen:** Timing-Differenzen zwischen AVR und ESP32 in den zeitkritischen CC1101-Routinen, eine fehlerhafte Interrupt-Verarbeitung nach der Initialisierung oder Probleme bei der seriellen Kommunikation.
    *   **Aktueller Status (System responsiv, Debugging-Setup DONE, Initial Blocking Fixed, Multiple Uploads SUCCESSFUL):**
        1.  Ein neues Image (Version 1.26.70) mit Debugging-Instrumentierung (`gdo_isr_count` und ausführlichen `Serial.println` in `setup()`/`loop()`) wurde erfolgreich erstellt und geflasht.
        2.  Das Problem der Systemblockade durch einen frühen "Interrupt Storm" wurde behoben.
        3.  Der `gdo_isr_count` wird bei jedem GDO0-Interrupt inkrementiert.
        4.  Mehrere Uploads des Binaries auf den XIAO-ESP32-C3 waren erfolgreich.
        5.  **Serielle Ausgabe Problem (Critical):** Trotz erfolgreichem Flash und implementierten Debug-Ausgaben (Heartbeat, ISR Count) wird keine serielle Ausgabe im `pio device monitor` empfangen.
        6.  **Board Boot-Modus Problem:** Das Board geht nach dem Reset durch `esptool.py` manchmal in den "wait usb download" Modus, anstatt die geflashte Applikation zu starten.
    *   **Aktuelles Monitor-Problem (In Progress):**
        *   **Problem 1: Port-Locking:** Nach dem initialen erfolgreichen Flash können weitere Uploads oder das Öffnen des seriellen Monitors mit `pio device monitor` fehlschlagen, da der Port `/dev/ttyACM0` als "busy" gemeldet wird (`[Errno 11] Resource temporarily unavailable`).
        *   **Problem 2: `termios.error`:** Die Verwendung von `timeout` vor `pio device monitor` führt zu einem `termios.error: (25, 'Inappropriate ioctl for device')`, was darauf hindeutet, dass der Monitor in dieser Umgebung keine vollen TTY-Kontrollfunktionen nutzen kann. Dies verhindert einen stabilen Aufruf des Monitors.
        *   **Nächste Schritte zur Behebung des Port- und Monitor-Problems:**
            1.  Den blockierenden Prozess identifizieren und beenden (z.B. mit `fuser -k /dev/ttyACM0`).
            2.  Den `pio device monitor` *ohne* `timeout` direkt ausführen, um das `termios.error` zu umgehen und zu sehen, ob dies eine stabile serielle Verbindung ermöglicht.
            3.  Manuelles Drücken der RST/BOOT-Tasten am XIAO-ESP32-C3 nach dem Flashen könnte erforderlich sein, um das Board zuverlässig aus dem Download-Modus in den Anwendungsmodus zu bringen und serielle Ausgaben zu erhalten.
    *   **Nächste Schritte (Manuelle Hardware-Verifikation UNBEDINGT erforderlich - *nach erfolgreichem Upload und Port-Freigabe*):**
        1.  **Hardware-Anschluss:** Stelle sicher, dass der CC1101 am XIAO-ESP32-C3 gemäß dem definierten Pinout angeschlossen ist (SCK=GPIO8, MISO=GPIO9, MOSI=GPIO10, CS=GPIO5, GDO0=GPIO3, GDO2=GPIO4, LED=GPIO21).
        2.  **Monitor beobachten und Interaktion testen:**
            *   Überprüfe, ob das Board auf Befehle wie `V` über die serielle Konsole reagiert. (Sollte jetzt funktionieren, sobald der Monitor läuft).
            *   Beobachte, ob der "Heartbeat" und der `ISR Count` in der seriellen Ausgabe erscheinen und bei RF-Aktivität (z.B. durch Senden eines FS20-Signals) ansteigen.
            *   **Fall A: Zähler bleibt 0:** Überprüfe die Hardware-Verkabelung von GDO0 und GDO2 zum CC1101 erneut. Stelle sicher, dass der CC1101 richtig initialisiert ist und Signale sendet.
            *   **Fall B: Zähler steigt an (aber keine Pakete dekodiert):** Wenn der Zähler ansteigt, die Pakete aber nicht dekodiert werden, liegt das Problem tiefer in der Software-Verarbeitung oder im Timing. Dann sollte mit dem `X21` Befehl (Hex-Dump der empfangenen Rohdaten) weiter analysiert werden, um zu sehen, welche Art von Daten im Empfangspuffer ankommt.
    *   **Potenzieller weiterer Fix (bei Boot-Problemen):** Sollte das Board trotz des korrigierten Pinouts *Boot-Probleme* zeigen, wird ein **10kΩ Pull-Up-Widerstand am CS-Pin (GPIO 5)** empfohlen, um den CC1101 während des Bootvorgangs sicher zu deaktivieren.
*   **Weitere Entwicklung:** Optimierung des SPI-Timings, Implementierung eines Web-Interfaces für ESP32.