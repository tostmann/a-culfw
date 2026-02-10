# a-culfw for PlatformIO

This repository contains a port of the well-known *a-culfw* (alternative firmware for CUL devices) to the **PlatformIO** build system.

## Aim (Intent)

The goal of this project is to modernize and simplify the development and compilation of the firmware. Instead of relying on complex, hand-written `Makefiles`, this project uses PlatformIO for clean management of dependencies and hardware variants.

**Advantages:**
* **One-Click-Setup:** Automatic installation of the correct toolchains (AVR, STM32, ESP32).
* **Centralized Management:** All finished firmwares are stored uniformly in the `/binaries` folder in the git root (one level above the code).
* **Automation:** A manifest (`manifest.json`) catalogues all builds including version, date, and hardware platform.
* **Modernization:** Support for modern hardware like the **XIAO ESP32-C3** (RISC-V).

## Supported Hardware

Currently, the following environments are defined:

1.  **nanoCUL868 / nanoCUL433** (ATmega328P)
2.  **CUL_V3** (ATmega32U4 with native USB)
3.  **MapleCUN** (STM32F103CB, Quad-CC1101, Ethernet)
4.  **XIAO-ESP32-C3** (ESP32-C3 RISC-V Native Port - Fully Functional)
5.  **ESP32-C3 / ESP32-C6** (Generic ESP32 Support)

## Generating Firmware (Build)

By default, `pio run` only builds the stable AVR targets. ESP32 targets must be selected explicitly.

### Build Commands in the Terminal

```bash
# Build only standard devices (AVR)
pio run

# Build a specific device (including ESP32) explicitly
pio run -e nanoCUL868
pio run -e XIAO_ESP32C3
```

### The Result
After a successful build, all relevant files are located in the `/binaries` directory in the git root:
* `manifest.json`: Contains metadata for all built versions.
* `*.hex`: Firmware for AVR-based CULs.
* `*.bin`: Firmware for STM32 or ESP32 (OTA and factory images).

## Flashing Firmware

### 1. nanoCUL (Arduino Nano)
```bash
avrdude -p atmega328p -c arduino -P /dev/ttyUSB0 -b 57600 -D -U flash:w:../binaries/nanoCUL868.hex:i
```

### 2. CUL V3 (ATmega32U4)
The CUL V3 must be in bootloader mode.
* **Web Flasher (Recommended):** Use the [busware CUL Flasher](https://prov.busware.de/culflasher/) to transfer the `.hex` file directly from the browser.
* **Manual (Linux/Mac):**
  ```bash
  dfu-programmer atmega32u4 erase
  dfu-programmer atmega32u4 flash ../binaries/CUL_V3.hex
  dfu-programmer atmega32u4 reset
  ```

### 3. XIAO ESP32-C3
```bash
pio run -e XIAO_ESP32C3 -t upload
```

## Repository Structure & Git

To keep the repository clean, only the final products in the `binaries/` folder and the core source code are tracked. Diagnostic tools and temporary build files are ignored or moved to `tools/`.

**Git Whitelist Principle:**
Only `.hex`, `.bin` and the `manifest.json` in the `/binaries/` folder are explicitly allowed in the `.gitignore`. All other artifacts in the `.pio/` folder remain local.

## Version Numbering

This project uses a dynamic versioning script:
* The **Base Version** is administered in `version.h`.
* The **Build Number** is automatically incremented (locally or via CI/CD).
* The `collect_binaries.py` script extracts this version and writes it directly into the global manifest.
