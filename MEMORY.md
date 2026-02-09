```markdown
# CULFW32 Project Status

## 1. General Project Information
*   **Project Name:** CULFW32 (Portierung der a-culfw auf 32-Bit-Mikrocontroller)
*   **Repository:** `tostmann/a-culfw.git` (forked)
*   **Local Codebase:** `/opt/ai_builder_data/users/763684262/projects/CULFW32/culfw/`
*   **Build System:** PlatformIO
*   **Target Architectures:** AVR, STM32, ESP32 (RISC-V)

## 2. Supported Boards & Firmware Versions
*   **Firmware Version:** 1.26.55 (latest build, automatisch inkrementiert - AI konnte das letzte XIAO-Image mit korrigiertem Pinout aufgrund Build-Problemen nicht selbst erstellen)
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
    *   **Aktuelles Pinout (Empfehlung für stabile Boots):** LED=GPIO21 (Pin D6), SPI: SCK=GPIO8 (Pin D8), MISO=GPIO9 (Pin D9), MOSI=GPIO10 (Pin D10), CS=GPIO5 (Pin D3), GDO0=GPIO3 (Pin D1), GDO2=GPIO4 (Pin D2).

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
*   **XIAO-ESP32-C3 Boot-Problem mit CC1101 (Aktive Untersuchung):**
    *   **Problem:** Das XIAO-ESP32-C3-Board bootet nicht zuverlässig, wenn der CC1101 vollständig verkabelt ist (insbesondere mit angeschlossenen CS/GDO-Pins), was zu Abstürzen wie `Guru Meditation Error: Instruction access fault` führt.
    *   **Vermutete Ursache:** Interferenz des CC1101 mit den "Strapping Pins" des ESP32-C3 während des Bootvorgangs. Speziell GPIO 9 (MISO) ist ein kritischer Strapping-Pin, der beim Booten auf HIGH liegen muss. Wenn der CC1101 durch einen nicht initialisierten CS-Pin aktiviert wird, kann er die MISO-Leitung auf LOW ziehen und den ESP32 in einen falschen Boot-Modus zwingen. Auch GPIO 2 ist ein kritischer Strapping-Pin.
    *   **Debug-Versuche:**
        *   Erster Versuch: CS auf GPIO 5 (Pin D3). Problem blieb bestehen.
        *   Zweiter Versuch: Komplette Verlegung der SPI-Pins (SCK=5, MISO=6, MOSI=7, CS=21) zur Vermeidung von GPIO 2, 8, 9. Problem blieb bestehen.
        *   Dritter Versuch (Aktueller Lösungsansatz): Beibehaltung der Standard-SPI-Pins (SCK=8, MISO=9, MOSI=10) da diese ohne angeschlossene Peripherie stabil funktionieren, und Verlegung der kritischen **CS-Leitung auf GPIO 5 (Pin D3)**. GDO0=GPIO3 (Pin D1), GDO2=GPIO4 (Pin D2), LED=GPIO21 (Pin D6).
    *   **KRITISCHES NEUES PROBLEM: PlatformIO Build-Fehler für XIAO-ESP32-C3:** Die automatische Generierung eines funktionierenden Factory-Images für das XIAO-ESP32-C3 mit der *korrekten Pin-Belegung* schlug aufgrund komplexer relativer Pfadprobleme in der `platformio.ini` fehl (Skript-Pfade, Include-Pfade, Quellverzeichnisse wurden nicht korrekt gefunden). Das Build-System wurde vorübergehend instabil, konnte aber auf einen Grundzustand zurückgesetzt werden.
    *   **Nächste Schritte (für Benutzer):**
        1.  **Manuelles Update der `platformio.ini`:** Ersetze den kompletten `[env:XIAO-ESP32-C3]`-Block in der Datei `/opt/ai_builder_data/users/763684262/projects/CULFW32/culfw/Devices/ESP32/platformio.ini` mit dem von der AI bereitgestellten, korrigierten Codeblock (dieser enthält das sichere Pinout und die angepassten Build-Pfade).
        2.  **Manuelles Builden:** Starte den Build für das XIAO-ESP32-C3 Target lokal (z.B. mit `platformio run -e XIAO-ESP32-C3`).
        3.  **Umverdrahtung der Hardware:** Schließe den CC1101 am XIAO-ESP32-C3 gemäß dem **letzten empfohlenen Pinout** an: SCK=GPIO8, MISO=GPIO9, MOSI=GPIO10, **CS=GPIO5**, GDO0=GPIO3, GDO2=GPIO4, LED=GPIO21.
        4.  **Flashen des Images:** Flashe das manuell erstellte `XIAO-ESP32-C3-factory.bin` auf das Board.
        5.  **Test des Bootvorgangs:** Überprüfe, ob das Board nun stabil bootet.
    *   **Potenzieller weiterer Fix:** Sollte das Problem weiterhin bestehen, wird ein **10kΩ Pull-Up-Widerstand am CS-Pin (GPIO 5)** empfohlen, um den CC1101 während des Bootvorgangs sicher zu deaktivieren.
*   **CC1101 Funktionstests (Pending):** Sobald das XIAO-ESP32-C3 zuverlässig mit angeschlossener Hardware bootet, werden vollständige Funktionstests des CC1101 (Befehle `C` für Registerausgabe, `V` für Versions-/Frequenzprüfung, `X21` für FS20-Empfang) durchgeführt.
*   **Weitere Entwicklung:** Optimierung des SPI-Timings, Implementierung eines Web-Interfaces für ESP32.
```