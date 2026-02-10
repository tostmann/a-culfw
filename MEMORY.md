## 1. General Project Information
*   **Project Name:** CULFW32 (Portierung der a-culfw auf 32-Bit-Mikrocontroller)
*   **Repository:** `tostmann/a-culfw.git` (forked)
*   **Local Codebase:** `/opt/ai_builder_data/users/763684262/projects/CULFW32/culfw/`
*   **Build System:** PlatformIO
*   **Target Architectures:** AVR, STM32, ESP32 (RISC-V)

## 2. Supported Boards & Firmware Versions
*   **Firmware Version:** 1.26.140 (latest build, automatisch inkrementiert nach Fixes)
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
    *   **XIAO-ESP32-C3:** (MAX!/Moritz Functional, SlowRF-Puls-Erkennung stabil, **Getestet & Flash Erfolgreich**)
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
    *   `clock.c`: `ticks`-Variable für `USE_HAL`-Plattformen konditional kompiliert. The `ticks` variable is now conditionally compiled and defined only in `hal.cpp` when `USE_HAL` is active, centralizing its definition and resolving linker errors.
    *   `rf_receive.c`: CC1101-zeitkritische Routinen um `defined(ESP32) || defined(STM32)` erweitert.
    *   `fncollection.c`: Null-Pointer-Check in `eeprom_factory_reset`, `hal_eeprom_init()` für HAL-Plattformen integriert.
*   **ESP32 USB-CDC & Konsole (DONE):**
    *   `ARDUINO_USB_MODE=1` und `ARDUINO_USB_CDC_ON_BOOT=1` für direkte USB-Serial über `Serial`-Objekt aktiviert.
    *   Doppelte Initialisierungen im `setup()` wurden entfernt.
    *   `Serial` wird konditional zu `USBSerial` definiert, um sicherzustellen, dass die USB-CDC-Schnittstelle verwendet wird.
    *   `Serial.setTxTimeoutMs(0)` wurde für nicht-blockierende serielle Ausgaben hinzugefügt.
    *   Der vorherige Boot-Hänger nach Initialisierung der CC1101-Hardware wurde durch einen `delay(2000)` in `setup()` und die Anpassung der FreeRTOS-Task-Priorität des `background_task` auf 1 (niedriger als USB-Stack) behoben, was eine stabile serielle Kommunikation nach dem Bootvorgang ermöglicht.
*   **ESP32 Pointer-Handling Fix (DONE):** `ttydata.c` korrigiert, um 32-Bit-Funktionspointer auf ESP32/ARM korrekt zu behandeln (Behebung des `Instruction access fault`-Absturzes, verursacht durch 16-Bit-AVR-Makro `__LPM_word` beim Auslesen der `fntab`).
*   **Flexible Pin-Konfiguration (DONE):** `board.h` (für ESP32) verwendet jetzt `#ifndef`-Guards, wodurch Pin-Definitionen über `build_flags` in `platformio.ini` für spezifische Board-Targets überschrieben werden können. `DBOARD_NAME` and `DBOARD_ID_STR` flags were added to `platformio.ini`.
*   **PlatformIO Build-System-Fixes & Pfad-Standardisierung für ESP32-Targets (DONE):**
    *   Die globale `src_dir = .` Einstellung wurde in die `[platformio]` Sektion der Haupt-`platformio.ini` (`culfw/platformio.ini`) verschoben, um das Projekt-Root-Verzeichnis als Quellverzeichnis für alle Targets zu definieren.
    *   Alle `build_flags` für Include-Pfade (`-I`) und `build_src_filter` in `culfw/Devices/ESP32/platformio.ini` wurden für die ESP32-Targets (ESP32-C3, XIAO-ESP32-C3, ESP32-C6) auf korrekte relative Pfade zum Projekt-Root (`./`, `clib`, `ESP32` etc.) angepasst.
    *   Dies ermöglicht nun den erfolgreichen Build aller ESP32-Targets (`ESP32-C3`, `XIAO-ESP32-C3`, `ESP32-C6`) aus dem Projekt-Root (`culfw/`) mit dem Kommando `pio run -d culfw/ -e <env_name>`.
    *   Alle ESP32-Targets bauen nun erfolgreich und generieren sowohl `factory.bin` als auch `app.bin`.
*   **AVR-spezifische Header-Dateien für ESP32 (DONE):** Die `#include <avr/io.h>` und `#include <avr/pgmspace.h>` Pfade wurden in den `build_flags` für ESP32-Targets korrekt referenziert, um Kompilierungsfehler zu beheben.
*   **GDO-Interrupt-Handling & SlowRF-Puls-Analyse für ESP32 (DONE):** Die zeitkritische Timer-Logik wurde von einem blockierenden Hardware-Interrupt auf eine effiziente Polling-Methode (`hal_timer_task`) innerhalb eines dedizierten FreeRTOS-Background-Tasks (`background_task`) verschoben. Der GDO0-Interrupt (`gdo_interrupt_handler`) ist nun minimalistisch (`IRAM_ATTR`) und inkrementiert primär einen Zähler, während er den RF-Hardware-Timer zurücksetzt. Die präzise Erfassung der Pulsbreiten erfolgt mittels `esp_timer_get_time()` in `CC1100_in_callback` und die aufwendigere Analyse (`RfAnalyze_Task`) in dem neuen Hintergrund-Task. Der `gdo_interrupt_handler` nun auch den `rf_hw_timer` zurückgesetzt, um genaue Silence-Erkennung zu gewährleisten. Die `CC1100_in_callback` Funktion erfasst jetzt die Pulsdauern noch präziser, indem sie `esp_timer_get_time()` verwendet und `last_isr_time_val` speichert, um `hightime` und `lowtime` direkt aus den Zeitdifferenzen zu berechnen und so Jitter zu eliminieren. Interne Pull-up-Widerstände am `GDO0_PIN` werden explizit deaktiviert (`INPUT_PULLDOWN`) und die SPI-Frequenz auf 1MHz begrenzt, um die Signalintegrität zu verbessern. Dies verhindert Watchdog-Timeouts und verbessert die Stabilität. Verifizierung: `gdo_interrupt_handler` wird korrekt ausgelöst, und die High/Low-Flanken werden gezählt (z.B. 198H/180L bei FS20-Sendungen), was die korrekte Hardware-Anbindung des CC1101 (GDO0) bestätigt.
*   **Git-Integration (DONE):** Projektänderungen (alle ESP32-Fixes, C3/C6 und XIAO-Targets) in einem neuen Feature-Branch (`feature/esp32-support`) committet und erfolgreich via SSH nach GitHub gepusst.
*   **PlatformIO Core Installation (DONE):** PlatformIO Core wurde erfolgreich auf dem System unter `/root/.platformio/penv/bin/pio` installiert und konfiguriert, um Uploads und Builds zu ermöglichen.
*   **ESP32 C Standard Library Conflicts (DONE - `_BV` warning persists):** Die `time_t` und `struct tm` Redefinitionen wurden behoben. Die `sei()` und `cli()` redefinition warnings wurden durch `#ifndef` Guards in `ESP32/avr/avr/interrupt.h` behoben. Die `_BV` redefinition warning bleibt weiterhin bestehen. Warnings `initialized and declared 'extern'` for `ticks`, `SREG`, `TIMSK0` in `ESP32/hal.cpp` remain.
*   **CLI Serial Test Script (`cul_exchange_test.py`, `cul_fifo_test.py`, `slowrf_test.py`, `check_serial.py`) (DONE):** Python-basierte Skripte wurden entwickelt und erfolgreich zur Diagnose und Funktionsprüfung verwendet.
*   **XIAO-ESP32-C3 Pin-Re-Assignment (DONE):** Ursache (CONFIRMED): Strapping-Pin-Conflicte (insbesondere GPIO 8, 9 und potenziell 2) verhinderte den Applikationsstart des XIAO-ESP32-C3. Fix: Das Pinout für den XIAO-ESP32-C3 wurde angepasst, um die Verwendung der kritischen Strapping-Pins für SPI und GDO zu vermeiden. Verifizierung: Erfolgreiche Kommunikation mit dem CC1101 und korrektes Booten bestätigen die Funktionalität.
*   **`ttydata.h` Function Pointer Type Fix (DONE):** Der Typ des Funktionszeigers `fn` innerhalb der `t_fntab` Struktur in `culfw/clib/ttydata.h` wurde von `void (* const fn)(char *)` auf `void (* const fn)(uint8_t *)` geändert. Ein anschließender `invalid conversion` Compilerfehler in `main.cpp` wurde durch einen Cast behoben.
*   **`cc1100.c` Serial Output Conditional Compilation (DONE):** `Serial.println` Aufrufe in `culfw/clib/cc1100.c` wurden angepasst, um Konflikte mit der `Serial` Objektdefinition zu vermeiden.
*   **`eeprom_factory_reset` (command `e`) Fix (DONE):** Ein Absturz beim Ausführen des `e` Befehls wurde behoben, indem `esp_restart()` für ESP32-Targets korrekt in `fncollection.c` implementiert wurde.
*   **GDO Pin Mapping für Protokoll-spezifische Modi (DONE):** Die CC1101-Registerkonfigurationen (`MORITZ_CFG`, `ASKSIN_CFG`) in `clib/rf_moritz.c` und `clib/rf_asksin.c` wurden angepasst, um den "Packet Received"-Status über `IOCFG0` (GDO0, Register `0x02`) auszugeben. Die Standard-SlowRF-Konfiguration (`CC1100_CFG`) in `clib/cc1100.c` wurde auf `IOCFG0 = 0x0D` (Serial Data Output) umgestellt, um das Bit-Sampling in `rf_receive.c` zu ermöglichen.
*   **TTY Buffer Size (DONE):** Die Größe des TTY-Buffers wurde in den ESP32-spezifischen `board.h` Definitionen (und assoziierten Dateien) von 128 auf 1024 Bytes erhöht. Die `rb_t` Struktur in `ringbuffer.h` verwendet nun `uint16_t` für die Pointer, um Puffer > 255 Bytes zu unterstützen und bei hohem Funkaufkommen keine Daten zu verlieren.
*   **IRAM_ATTR Optimierung (DONE):** Kritische Funktionen wie `addbit`, `makeavg` in `rf_receive_bucket.c`, `CC1100_in_callback` in `hal.cpp`, `gdo_interrupt_handler`, `onTickTimer`, `onRfTimer`, `hal_enable_CC_timer_int`, `rf_receive_TimerElapsedCallback` wurden mit `IRAM_ATTR` markiert, um Latenzen durch Flash-Zugriffe während der Interrupt-Verarbeitung zu minimieren.
*   **FreeRTOS Task-based RF Analysis (DONE):** Ein dedizierter FreeRTOS-Task (`background_task`) mit Priorität 1 (niedriger als USB-Stack) wurde implementiert, der `hal_timer_task()` und `RfAnalyze_Task()` in einer Endlosschleife ausführt. Dies ermöglicht eine entkoppelte und stabile Verarbeitung der RF-Daten unabhängig von der USB-Seriellen Kommunikation in `loop()`.
*   **System Timer Frequency (DONE):** Die System-Heartbeat-Variable `ticks` und der Aufruf von `Minute_Task()` werden nun mit 125Hz (alle 8ms) aktualisiert, um die ursprünglichen CULFW-Timing-Erwartungen zu erfüllen.
*   **Enhanced Debugging (DONE):** Command `y`: Der `y`-Befehl wurde implementiert, der `show_debug()` aufruft, um aktuelle Debug-Informationen wie den Zähler der GDO0-Interrupts (`GDO0_INT_COUNT`), aufgeschlüsselt in High- und Low-Flanken (`ISR H/L/T`), System `ticks` und den Füllstand des RF-Receive-Buckets (`bucket_nrused`) über die serielle Konsole auszugeben. `display_debug` wurde zu `show_debug` umbenannt, um Konflikte zu vermeiden. Bit Collection Output: Logging innerhalb von `addbit` wurde aktiviert, um `0` oder `1` für erkannte Bits anzuzeigen (aktiviert mit `X3F` Monitor-Modus). Der `X3F`-Raw-Monitor-Output zeigt nun explizit `S` (Sync), `C` (Collect), `R` (Reset), `.` (Silence) und die neu gesammelten `0`/`1` Bits. Die Debug-Ausgabe in `CC1100_in_callback` wurde gestrafft, um den Fokus auf Schlüsselereignisse und die Bit-Sammlung zu legen.
*   **Improved Serial Output Speed (DONE):** Die `loop()`-Logik zum Schreiben des `TTY_Tx_Buffer` an `Serial` wurde optimiert, um so viele Bytes wie möglich in einem Schritt zu schreiben (`Serial.availableForWrite()`), was die Responsivität der Debug-Ausgabe verbessert.
*   **Global `bucket_nrused` (DONE):** Das `bucket_nrused` Array, das den Füllstand der RF-Receive-Buckets angibt, wurde global zugänglich gemacht, um seinen Status über den Debug-Befehl `y` zu überwachen.
*   **Full Microsecond Precision & Robustness for SlowRF Timing (DONE - Refined):** Das `TSCALE`-Makro in `rf_receive_bucket.h` wurde dauerhaft von `(x/16)` auf `(x)` geändert, wodurch der 1/16 Skalierungsfaktor für 32-Bit-Plattformen entfällt. Alle relevanten Timing-Variablen (`hightime`, `lowtime`, `wave_t`-Elemente) und Funktionsparameter (`wave_equals`, `makeavg`) wurden auf `pulse_t` (uint16_t) oder `uint32_t` umgestellt, um die unskalierten Mikrosekundenwerte ohne Überlauf zu verarbeiten. Ein kritischer Typkonflikt in `wave_equals` (zuvor eine Mischung aus `uint8_t` und `pulse_t`) wurde behoben. Die `>> 4` Skalierung wurde explizit aus den `hightime`/`lowtime`-Berechnungen in `rf_receive.c` entfernt. Die `makeavg`-Funktion in `rf_receive_bucket.c` wurde aktualisiert, um `pulse_t`-Typen zu akzeptieren und verwendet eine präzisere `((uint32_t)i * 3 + j) / 4`-Berechnung, um Rundungsfehler zu vermeiden und Überläufe bei größeren Mikrosekundenwerten zu verhindern. `TDIFF` (Toleranz) wurde auf 120µs (von 160µs) angepasst. Die Berechnung von `hightime` und `lowtime` in `CC1100_in_callback` und `gdo_interrupt_handler` in `hal.cpp` wurde verfeinert, um `static volatile pulse_t last_isr_time_val = 0;` und `esp_timer_get_time()` für jitterfreie Mikrosekundenpräzision zu verwenden. Compilerwarnungen im Zusammenhang mit `volatile pulse_t *` für `calcOcrValue`-Argumente wurden durch Type-Casting behoben. **Neueste Verfeinerungen:** `culfw/ESP32/hal.cpp`: Die Funktion `HAL_timer_get_counter_value` wurde angepasst, um `esp_timer_get_time()` direkt zu verwenden und einen präzisen, jitterfreien Mikrosekunden-Timestamp zu liefern. `HAL_timer_reset_counter_value` wurde ebenfalls entsprechend angepasst, um den `rf_counter_offset` auf Basis des aktuellen ISR-Timestamps (`last_isr_time_val`) zu setzen. `culfw/clib/rf_receive.c`: Die Variable `c` in `CC1100_in_callback` wurde auf `uint32_t` geändert, um potenzielle 16-Bit-Überläufe bei der Messung von Pulslängen, insbesondere bei langen "Silence"-Perioden, zu verhindern. `culfw/clib/rf_receive.c`: Die `reset_input()`-Funktion wurde erweitert, um `hightime[CC_INSTANCE]` und `lowtime[CC_INSTANCE]` explizit auf `0` zurückzusetzen. Dies verhindert, dass veraltete oder fehlerhafte Zeitwerte aus früheren, unvollständigen Paketen die Erkennung neuer Sync-Sequenzen stören.
*   **Concurrency for Shared RF Receive Variables (DONE):** Die Variablen `bucket_in`, `bucket_out` und `bucket_nrused` in `rf_receive.c` (und `rf_receive_bucket.h`) wurden explizit als `volatile` markiert, um einen korrekten und atomaren Zugriff über verschiedene FreeRTOS-Tasks (ISR, `background_task`, `RfAnalyze_Task`) hinweg zu gewährleisten.
*   **FS20/FHT Protocol Trailing Bit Handling (DONE):** In `RfAnalyze_Task()` innerhalb von `rf_receive.c` wurde vor dem Aufruf von `analyze(b, TYPE_FS20, &oby)` für `TYPE_FS20` ein bedingter manueller `addbit`-Aufruf hinzugefügt. Dies berücksichtigt Szenarien, in denen die fallende Flanke des letzten Bits eines Pakets verpasst werden könnte, und verbessert so die Vollständigkeit der erfassten Pakete. Der gleiche Mechanismus wurde auch für `TYPE_KS300` angewendet.
*   **Robustness of `analyze()` Function (DONE):** Interne Zähler (`cnt`, `max`) innerhalb der `analyze()`-Funktion in `rf_receive.c` wurden auf `uint16_t` geändert, um potenzielle Überläufe bei der Verarbeitung längerer Pakete zu verhindern.
*   **Enhanced Protocol Debugging (Parity Error Reporting) (DONE):** Die `analyze()`-Funktion in `rf_receive.c` wurde erweitert, um eine explizite Debug-Ausgabe (`P<byte_index><hex_byte>?<expected_parity>`) zu liefern, wenn ein Paritätsfehler bei FS20/FHT-Paketen erkannt wird. Dies unterstützt die präzise Identifizierung von Datenbeschädigungen oder Fehlern in der Paritätsberechnung.
*   **Increased RF Receive Bucket Capacity (DONE):** Das `RCV_BUCKETS`-Makro (Anzahl der Empfangs-Buckets) wurde in `rf_receive.c` und `rf_receive_bucket.h` von 8 auf 16 erhöht, um eine größere Pufferkapazität für eingehende RF-Pakete bereitzustellen und die Wahrscheinlichkeit von Überläufen während Phasen hoher Aktivität zu reduzieren.
*   **SlowRF Protocol Decoding (FS20/FHT Functional) (DONE):**
    *   Das `SILENCE`-Makro in `culfw/clib/rf_receive_bucket.h` wurde von `15000` auf `50000` Mikrosekunden erhöht, um vorzeitige Timeouts während der Paketverarbeitung zu verhindern.
    *   Die Timer-Startsequenz in `hal_enable_CC_timer_int` (`culfw/ESP32/hal.cpp`) wurde angepasst (`timerAlarmWrite`, `timerAlarmEnable`, `timerWrite(rf_hw_timer, 0)`) für ein zuverlässiges One-Shot-Timer-Triggering.
    *   Anpassungen in `culfw/clib/rf_receive.c` (in `CC1100_in_callback` im `STATE_COLLECT`-Zweig) um `wave_equals` mit `hightime[CC_INSTANCE]` und `lowtime[CC_INSTANCE]` für die initiale Bit-Erkennung korrekt zu verwenden.
    *   FS20/FHT-Pakete werden nun erfolgreich dekodiert und im `X3F`-Monitor-Modus mit Bit-Streams angezeigt (z.B. `F66666600E1`).

## 4. Known Issues & Next Steps
*   **RF-Datenaustausch (Slow RF - FS20, Intertechno) (DONE):**
    *   **Problem gelöst:** Die vorherigen Probleme mit dem Abbruch in `STATE_COLLECT` für SlowRF-Protokolle (FS20/FHT) wurden behoben. Die Systeme erkennen nun Sync-Sequenzen, sammeln Bits und dekodieren erfolgreich vollständige Pakete.
    *   **MAX!/Moritz:** Bidirektionaler Datenaustausch funktioniert einwandfrei.
    *   **Intertechno:** Die grundlegende Protokollunterstützung ist implementiert und aktiviert, muss aber noch mit realer Hardware verifiziert werden.
*   **Kompilierungsfehler und Warnungen (UNRESOLVED):**
    *   Die `_BV` redefinition warning (`ESP32/avr/avr/io.h` vs `Arduino.h`) bleibt weiterhin bestehen und muss noch final adressiert werden.
    *   Warnungen `initialized and declared 'extern'` for `ticks`, `SREG`, `TIMSK0` in `ESP32/hal.cpp` bleiben bestehen und sollten korrigiert werden.
*   **Protokoll-Unterstützung (DONE):**
    *   Die Flags `HAS_IT`, `HAS_INTERTECHNO`, `HAS_HMS`, `HAS_ESA`, `HAS_TCM97001` wurden in `culfw/Devices/ESP32/board.h` für die ESP32-Targets aktiviert.
    *   Der `i`-Befehl für Intertechno wurde in der `fntab` (`culfw/Devices/ESP32/main.cpp`) korrekt mit `it_func` verknüpft.

*   **Weitere Entwicklung:**
    *   **Priorität 1: Optimierung und Validierung des SlowRF-Empfangs.**
        *   **`SILENCE` Timeout-Optimierung:** Feinschliff des `SILENCE`-Wertes, um eine optimale Balance zwischen Paketerkennung und minimaler Latenz zu finden (aktuell 50ms).
        *   **Filter Re-Aktivierung:** Re-Aktivierung und Validierung des `delbit(b)`-Aufrufs und des `checkForRepeatedPackage` Filters in `rf_receive.c` zur Vermeidung doppelter oder fehlerhafter Paketmeldungen.
        *   **Langzeit-Stabilitätstests:** Durchführung umfangreicher Tests zur Erkennung von Paketverlusten oder Instabilitäten unter hoher Funklast über längere Zeiträume.
    *   **Priorität 2: Finalisierung der Kompilierungswarnungen.**
    *   **Langfristig:** Optimierung des SPI-Timings, Implementierung eines Web-Interfaces für ESP32.