# a-culfw für PlatformIO

Dieses Repository enthält eine Portierung der bekannten **a-culfw** (alternative Firmware für CUL-Geräte) auf das **PlatformIO** Build-System.

## Absicht (Intent)

Das Ziel dieses Projekts ist es, die Entwicklung und Kompilierung der Firmware zu modernisieren und zu vereinfachen. Anstatt auf komplexe, handgeschriebene `Makefiles` und manuell installierte Toolchains angewiesen zu sein, nutzt dieses Projekt PlatformIO.

**Vorteile:**
* **One-Click-Setup:** Automatische Installation der korrekten AVR- und STM32-Toolchains.
* **IDE-Integration:** Volle Unterstützung in VSCode (IntelliSense, Code-Navigation).
* **Modularität:** Saubere Trennung der Konfigurationen für verschiedene Hardware-Varianten.
* **Automatisierung:** Automatische Versionierung (Build-Nummer & Datum) und Sortierung der Firmware-Dateien.

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
4.  **`MapleCUN`** (STM32)
    * Basis: STM32F103CB (z.B. Maple Mini Clone)
    * Varianten:
        * `MapleCUNx4_W5100_BL` (Wiznet W5100 Ethernet)
        * `MapleCUNx4_W5500_BL` (Wiznet W5500 Ethernet)
    * Feature: 4x CC1101 Transceiver Support, Ethernet

## Voraussetzungen

1.  Installiere [Visual Studio Code (VSCode)](https://code.visualstudio.com/).
2.  Installiere die **PlatformIO IDE** Erweiterung in VSCode.
3.  Klone dieses Repository oder lade es herunter.
4.  Öffne den Ordner in VSCode ("Datei" -> "Ordner öffnen...").

## Firmware generieren (Build)

PlatformIO kümmert sich um alles. Es gibt zwei Wege, die Firmware zu bauen:

### Weg A: Über die Seitenleiste (Empfohlen)
1.  Klicke links auf das **Alien-Symbol** (PlatformIO).
2.  Wähle unter **Project Tasks** das gewünschte Gerät aus (z.B. `MapleCUNx4_W5500_BL`).
3.  Klicke auf **Build**.

### Weg B: Über das Terminal
Öffne ein Terminal in VSCode und führe folgenden Befehl aus:

    # Alles bauen
    pio run

    # Nur ein bestimmtes Gerät bauen
    pio run -e nanoCUL868
    pio run -e CUL_V3
    pio run -e MapleCUNx4_W5500_BL

### Das Ergebnis
Nach einem erfolgreichen Build passiert Folgendes automatisch:
1.  Die Version in der Firmware wird hochgezählt.
2.  Die fertige Firmware-Datei (`.hex` für AVR, `.bin` für STM32) wird in das passende Unterverzeichnis kopiert:
    * `Devices/nanoCUL/nanoCUL868.hex`
    * `Devices/CUL/CUL_V3.hex`
    * `Devices/MapleCUN/MapleCUNx4_W5500_BL.bin`

## Firmware flashen

Je nach Hardware und Bootloader gibt es unterschiedliche Methoden.

### 1. nanoCUL (Arduino Nano)
Der nanoCUL nutzt meist den Standard Arduino-Bootloader.

**Voraussetzung:** Der Nano muss per USB angeschlossen sein. Finde den Port heraus (Linux: `/dev/ttyUSB0`, Windows: `COM3`).

    avrdude -p atmega328p -c arduino -P /dev/ttyUSB0 -b 57600 -D -U flash:w:Devices/nanoCUL/nanoCUL868.hex:i

### 2. CUL V3 (ATmega32U4)
Der CUL V3 muss sich im **Bootloader-Modus** befinden (Taster beim Einstecken gedrückt halten).

**Linux / Mac (`dfu-programmer`):**

    dfu-programmer atmega32u4 erase
    dfu-programmer atmega32u4 flash Devices/CUL/CUL_V3.hex
    dfu-programmer atmega32u4 reset

**Windows:**
Nutze das Tool **Atmel FLIP** (siehe Doku).

### 3. MapleCUN (STM32F103)
Für den STM32 wird der **Maple Bootloader (2.0)** vorausgesetzt. Die Firmware wird per DFU (Device Firmware Upgrade) übertragen.

**Schritt 1: DFU-Modus aktivieren**
1.  Reset-Taste drücken und halten.
2.  User-Button (meist "BUT" oder "HWB") drücken und halten.
3.  Reset-Taste loslassen (User-Button weiter halten).
4.  User-Button loslassen.
5.  Die LED sollte nun **schnell blinken** (Perpetual Bootloader Mode).

**Schritt 2: Flashen**

**Option A: Direkt über PlatformIO** (wenn `dfu-util` Treiber installiert sind):

    pio run -e MapleCUNx4_W5500_BL -t upload

**Option B: Manuell mit `dfu-util`:**
Der Maple Bootloader nutzt meist `alt 2` für den Flash-Speicher (ab 0x8005000).

    dfu-util -a 2 -D Devices/MapleCUN/MapleCUNx4_W5500_BL.bin -R

## Versionsnummerierung

Das Projekt nutzt ein automatisches Versioning-Skript.
* Die **Basis-Version** (z.B. 1.26) wird in `version.h` gepflegt.
* Die **Build-Nummer** (z.B. 42) wird lokal in `.build_number` (im Root) hochgezählt.
* Beim Kompilieren wird die Firmware automatisch als `1.26.42` getaggt.
* Zum Zurücksetzen des Zählers einfach die Datei `.build_number` löschen.
