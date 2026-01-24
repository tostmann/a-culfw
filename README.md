# a-culfw für PlatformIO

Dieses Repository enthält eine Portierung der bekannten **a-culfw** (alternative Firmware für CUL-Geräte) auf das **PlatformIO** Build-System.

## Absicht (Intent)

Das Ziel dieses Projekts ist es, die Entwicklung und Kompilierung der Firmware zu modernisieren und zu vereinfachen. Anstatt auf komplexe, handgeschriebene `Makefiles` und manuell installierte Toolchains angewiesen zu sein, nutzt dieses Projekt PlatformIO.

**Vorteile:**
* **One-Click-Setup:** Automatische Installation der korrekten AVR-Toolchain und Compiler-Versionen.
* **IDE-Integration:** Volle Unterstützung in VSCode (IntelliSense, Code-Navigation).
* **Modularität:** Saubere Trennung der Konfigurationen für verschiedene Hardware-Varianten.
* **Automatisierung:** Automatische Versionierung (Build-Nummer & Datum) und Sortierung der Hex-Files.

## Unterstützte Hardware

Aktuell sind folgende Environments definiert:

1.  **`nanoCUL868`**
    * Basis: Arduino Nano (ATmega328P)
    * Frequenz: 868 MHz (CC1101 Modul)
2.  **`nanoCUL433`**
    * Basis: Arduino Nano (ATmega328P)
    * Frequenz: 433 MHz (CC1101 Modul)
3.  **`CUL_V3`**
    * Basis: CUL V3 Stick (ATmega32U4)
    * Feature: Native USB-Unterstützung (LUFA Stack)

## Voraussetzungen

1.  Installiere [Visual Studio Code (VSCode)](https://code.visualstudio.com/).
2.  Installiere die **PlatformIO IDE** Erweiterung in VSCode.
3.  Klone dieses Repository oder lade es herunter.
4.  Öffne den Ordner in VSCode ("Datei" -> "Ordner öffnen...").

## Firmware generieren (Build)

PlatformIO kümmert sich um alles. Es gibt zwei Wege, die Firmware zu bauen:

### Weg A: Über die Seitenleiste (Empfohlen)
1.  Klicke links auf das **Alien-Symbol** (PlatformIO).
2.  Wähle unter **Project Tasks** das gewünschte Gerät aus (z.B. `nanoCUL868`).
3.  Klicke auf **Build**.

### Weg B: Über das Terminal
Öffne ein Terminal in VSCode und führe folgenden Befehl aus:

    # Alles bauen
    pio run

    # Nur ein bestimmtes Gerät bauen
    pio run -e nanoCUL868
    pio run -e CUL_V3

### Das Ergebnis
Nach einem erfolgreichen Build passiert Folgendes automatisch:
1.  Die Version in der Firmware wird hochgezählt (sichtbar beim Booten des CUL).
2.  Die fertige `.hex` Datei wird automatisch in das passende Unterverzeichnis kopiert:
    * `Devices/nanoCUL/nanoCUL868.hex`
    * `Devices/nanoCUL/nanoCUL433.hex`
    * `Devices/CUL/CUL_V3.hex`

## Firmware flashen (Umgang mit Hex-Files)

Je nach Hardware und Bootloader gibt es unterschiedliche Methoden.

### 1. nanoCUL (Arduino Nano)
Der nanoCUL nutzt meist den Standard Arduino-Bootloader. Zum Flashen wird `avrdude` empfohlen.

**Voraussetzung:** Der Nano muss per USB angeschlossen sein. Finde den Port heraus (Linux: `/dev/ttyUSB0`, Windows: `COM3`).

    # Beispiel für nanoCUL 868 MHz an /dev/ttyUSB0
    avrdude -p atmega328p -c arduino -P /dev/ttyUSB0 -b 57600 -D -U flash:w:Devices/nanoCUL/nanoCUL868.hex:i

Parameter:
* `-p atmega328p`: Der Chip.
* `-c arduino`: Der Programmer (Bootloader).
* `-b 57600`: Die Baudrate (Nano Standard).

### 2. CUL V3 (ATmega32U4)
Der CUL V3 muss sich zum Flashen im **Bootloader-Modus** befinden.
* **Aktivieren:** Halte den Taster (HWB) auf der Rückseite gedrückt, während du den Stick in den USB-Port steckst. Die LED sollte blinken oder der Stick als neues Gerät erkannt werden.

Je nach installiertem Bootloader (Atmel DFU oder LUFA CDC) benötigst du unterschiedliche Tools.

#### Option A: Linux / Mac mit `dfu-programmer` (Empfohlen für Original Atmel Bootloader)
Dies ist der Standardweg für CULs mit dem nativen Atmel DFU-Bootloader.

Installation (Debian/Ubuntu):

    sudo apt-get install dfu-programmer

Flashen:

    # 1. Chip löschen (wichtig!)
    dfu-programmer atmega32u4 erase

    # 2. Neue Firmware schreiben
    dfu-programmer atmega32u4 flash Devices/CUL/CUL_V3.hex

    # 3. Bootloader verlassen / Neustart
    dfu-programmer atmega32u4 reset

#### Option B: Windows mit "Atmel FLIP" oder DFU-Tools
Unter Windows ist **Atmel FLIP** das offizielle Tool für den DFU-Bootloader.

1.  Lade **Atmel FLIP** herunter und installiere es (benötigt Java Runtime).
2.  Versetze den CUL in den Bootloader-Modus (Taster beim Einstecken).
3.  Falls Windows den Treiber nicht findet: Installiere manuell den Treiber aus dem FLIP-Installationsverzeichnis (`C:\Program Files (x86)\Atmel\Flip...\usb`).
4.  Starte FLIP:
    * Wähle Chip: **ATmega32U4**.
    * Klicke auf das USB-Kabel-Icon -> **Open**.
5.  Lade die Firmware:
    * File -> Load HEX File -> Wähle `Devices/CUL/CUL_V3.hex`.
6.  Flashen:
    * Setze Haken bei **Erase**, **Program**, **Verify**.
    * Klicke **Run**.
7.  Neustart:
    * Klicke **Start Application**.

#### Option C: `avrdude` (Nur für LUFA CDC Bootloader)
Falls auf dem CUL bereits ein *avr109*-kompatibler Bootloader (wie bei Arduino Leonardo oder einigen culfw-Varianten) installiert ist, funktioniert `avrdude`.

    # Port (/dev/ttyACM0) kann variieren!
    avrdude -p atmega32u4 -c avr109 -P /dev/ttyACM0 -b 57600 -U flash:w:Devices/CUL/CUL_V3.hex:i

## Versionsnummerierung

Das Projekt nutzt ein automatisches Versioning-Skript (`scripts/increment_version.py`).
* Die **Basis-Version** (z.B. 1.26) wird in `version.h` gepflegt.
* Die **Build-Nummer** (z.B. 42) wird lokal in `.build_number` (im Root) hochgezählt.
* Beim Kompilieren wird die Firmware automatisch als `1.26.42` getaggt und das Datum gesetzt.
* Zum Zurücksetzen des Zählers einfach die Datei `.build_number` löschen.
