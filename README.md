# a-culfw for PlatformIO

This repository contains a port of the well-known *a-culfw* (alternative firmware for CUL devices) to the **PlatformIO** build system.

Bug reports and questions about this version go here: [Issues](https://github.com/tostmann/a-culfw/issues) for problems, [Discussions](https://github.com/tostmann/a-culfw/discussions) for questions and ideas.

## Aim (Intent)

The goal of this project is to modernize and simplify the development and compilation of the firmware. Instead of relying on complex, hand-written `Makefiles`, this project uses PlatformIO for clean management of dependencies and hardware variants.

**Advantages:**
* **One-Click-Setup:** Automatic installation of the correct toolchains (AVR, STM32).
* **Centralized Management:** All finished firmwares are stored uniformly in the `/binaries` folder in the git root (one level above the code).
* **Automation:** A manifest (`manifest.json`) catalogues all builds including version, date, and hardware platform.

The port is partial on purpose: the hand written `makefile` in every
`culfw/Devices/*` directory still exists and is still the only way to build the
devices that have no PlatformIO environment yet.

## Supported Hardware

Currently, the following environments are defined:

1.  **nanoCUL868 / nanoCUL433** (ATmega328P)
2.  **CUL_V3** (ATmega32U4 with native USB)
3.  **MapleCUNx4_W5100_BL / MapleCUNx4_W5500_BL** (STM32F103CB, Quad-CC1101, Ethernet)

Every other device in `culfw/Devices/` — COC, CUN, CUNO/CUNO2, CUBe, SCC,
megaCUL, miniCUL, CUL-Arduino and the rest — is built with its own `makefile`,
not with PlatformIO.

There is no ESP32 target. A native port was started in January 2026 and removed
again in February.

Two toolchains are in play and they are not the same compiler: PlatformIO pins
`toolchain-atmelavr@~1.50400.0` (avr-gcc 5.4.0), while the makefiles use
whatever `avr-gcc` is on the PATH (verified against 14.2.0). Code size differs
measurably between the two.

## Generating Firmware (Build)

By default, `pio run` builds `CUL_V3` and `CUL_V3_DUDETTE`, the two images this
repository ships.
Other environments build only when selected explicitly.

### Build Commands in the Terminal

```bash
# Build the shipped images (default_envs)
pio run

# Build another environment explicitly
pio run -e nanoCUL868
```

### Build profiles

`TTYSBU` builds a CUL_V3 that talks over a UART on the USB-C SBU pins instead
of USB — PlatformIO environment `CUL_V3_SBU`. Wiring, the runtime mode
detection and what it costs are described in
[`culfw/Devices/CUL/SBU_SUPPORT.md`](culfw/Devices/CUL/SBU_SUPPORT.md).

`SLIM_HM_BUILD` builds a CUL_V3 for HomeMatic only (BidCos + HmIP) and drops
the protocols such a stick does not need. On CUL_V3_868MHZ that is 15736 instead
of 28202 bytes of flash and 992 instead of 2345 bytes of RAM:

```bash
cd culfw/Devices/CUL
make TARGET=CUL_V3 FREQUENCE=_868MHZ MCU=atmega32u4 \
     FLASH_SIZE=32768 BOOTLOADER_SIZE=4096 \
     EXTRA_CFLAGS=-DSLIM_HM_BUILD mostly_clean build size
```

`CUL_V3_DUDETTE` builds a CUL_V3 that installs firmware on devices with the
Dudette radio bootloader, from the
[web installer](https://install.busware.de/dudette/) or any other client of its
`D` commands. Its version line ends in `Dudette=1`. The module takes about 9.7 KB
of flash, so this build sits on `SLIM_HM_BUILD`: it is a firmware to flash with,
not one for everyday use. 868 MHz only. PlatformIO environment `CUL_V3_DUDETTE`,
or:

```bash
cd culfw/Devices/CUL
make TARGET=CUL_V3_DUDETTE FREQUENCE=_868MHZ MCU=atmega32u4 \
     FLASH_SIZE=32768 BOOTLOADER_SIZE=4096 mostly_clean build size
```

### The Result
A build copies each image it produces into the `/binaries` directory in the
git root and records it in `manifest.json`. Only `binaries/CUL_V3.hex`,
`binaries/CUL_V3_DUDETTE.hex` and `manifest.json` are part of the repository.

## Flashing Firmware

### 1. nanoCUL (Arduino Nano)
Build the image first (`pio run -e nanoCUL868`), it is not shipped.
```bash
avrdude -p atmega328p -c arduino -P /dev/ttyUSB0 -b 57600 -D -U flash:w:../binaries/nanoCUL868.hex:i
```

### 2. CUL V3 (ATmega32U4)
The CUL V3 must be in bootloader mode.
* **Web Flasher (Recommended):** Use the [busware CUL Flasher](https://install.busware.de/cul/) to transfer the `.hex` file directly from the browser.
* **Manual (Linux/Mac):**
  ```bash
  dfu-programmer atmega32u4 erase
  dfu-programmer atmega32u4 flash ../binaries/CUL_V3.hex
  dfu-programmer atmega32u4 reset
  ```

## Repository Structure & Git

To keep the repository clean, only the final products in the `binaries/` folder are tracked. Temporary build files are ignored.

**Git Whitelist Principle:**
Only `binaries/CUL_V3.hex`, `binaries/CUL_V3_DUDETTE.hex` and `binaries/manifest.json` are explicitly allowed in the `.gitignore`. Images of other environments and all artifacts in the `.pio/` folder remain local.

## Version Numbering

This project uses a dynamic versioning script:
* The **Base Version** is administered in `version.h`.
* The **Build Number** is automatically incremented (locally or via CI/CD).
* The `collect_binaries.py` script extracts this version and writes it directly into the global manifest.
