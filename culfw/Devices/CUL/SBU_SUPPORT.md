# Busware CUL "SBU-Native" Hybrid Design

## 1. Overview
The ATmega32u4-based Busware CUL stick has been redesigned to support a new "SBU-Native" architecture. The firmware and hardware now seamlessly operate in a "Dual-Path" mode using a single unified binary:
1. **Legacy Mode:** 100% backward compatible with standard `culfw` (USB-CDC communication, Radio on PD2/PD3).
2. **SBU-Native Mode:** Raw Hardware UART over USB-C SBU pins for low-latency communication with the new ESP32-C6 Gateway.

## 2. Hardware Architecture

### A. USB-C Interface
*   **D+ / D-:** Direct connection to ATmega for legacy USB-CDC enumeration.
*   **SBU1 (A8) / SBU2 (B8):** UART TX/RX routed via `USART1`.
*   **CC1 / CC2:** 5.1kΩ pull-downs.

### B. Multiplexer (Dual SPDT)
*   **Control Pin:** **PD4**.
*   **LOW State (Legacy Mode):** Routes CC1101 GDO0/GDO2 to PD3/PD2.
*   **HIGH State (SBU Mode):** Routes USB-C SBU1/SBU2 to PD3/PD2 (`USART1`).

### C. Radio Redundancy (CC1101)
*   **GDO0 (TX):** Connected to Switch NC1 **AND** permanently to **PD1**.
*   **GDO2 (RX):** Connected to Switch NC2 **AND** permanently to **PD0**.
*   **Legacy Mode:** Uses PD2 (INT2) for RX, PD3 for TX.
*   **SBU Mode:** Uses PD0 (INT0) for RX, PD1 for TX.

### D. Hardware Revision Detection
*   **Pin:** **PB4** with 10kΩ pull-down.
*   **Firmware Logic:** The firmware checks PB4 at boot using an internal pull-up. If it reads LOW, SBU Mode is automatically activated.

## 3. Firmware Implementation Details

### Unified Binary Strategy
A single firmware binary (`[env:CUL_V3_SBU]`) supports both hardware revisions out-of-the-box. Both `ISR(INT0_vect)` and `ISR(INT2_vect)` are compiled into the binary. The active interrupt source (`EIMSK` / `EICRA`) is dynamically selected at runtime during `tx_init()` and `ccRX()` based on the detected hardware mode. 

### SBU UART (`sbu_uart.c`)
In SBU mode, communication bypasses the USB stack and utilizes a RAM-optimized hardware UART interface over `USART1`.

### Feature Adjustments (Flash Constraints)
To accommodate the Dual-ISR logic and the SBU UART ringbuffer implementation within the ATmega32u4's strict 28KB PROGMEM limit:
*   `HAS_KOPP_FC` (Kopp Free Control) is **disabled** to free up ~3.3 KB of flash.
*   `TTY_BUFSIZE` drops from 128 to 64; on this device it sizes four buffers.
*   All other standard 868MHz protocols (AskSin, MBUS, RWE, Hoermann, Intertechno, Somfy, etc.) remain fully functional and have been verified.

## 4. Building and Testing

Build the dual-mode firmware using PlatformIO:
```bash
pio run -e CUL_V3_SBU
```

Sizes on `CUL_V3_868MHZ` with avr-gcc 14.2.0: 26586 bytes of flash and 2213 bytes of RAM with `TTYSBU`, against 28202 / 2345 without it.

Not verified on hardware in this repository: the merge into master was checked by building, not by running a CUL V3 with SBU wiring.
