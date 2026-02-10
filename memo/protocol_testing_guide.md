# CULFW32 Test Knowledge Base & Protocol Reference

This document serves as a reference for creating automated tests (Unit & Integration) for the CULFW32 project. It is derived from the official `commandref.html` and structured for systematic testing.

## 1. System Health & Basic Configuration
These commands verify the fundamental functionality of the firmware and microcontroller.

| Command | Parameter | Function / Test Goal | Expected RX (Return) | Critical for ESP32 |
| :--- | :--- | :--- | :--- | :--- |
| **`V`** | - | **Version:** Check responsiveness & firmware ID. | `V 1.xx <DeviceName> ...` | **YES** (Boot Check) |
| **`t`** | - | **Uptime:** Check internal timer (1/125s ticks). | Hex value (e.g. `01A2F`) | **YES** (Timer Stability) |
| **`e`** | - | **Factory Reset:** Reset EEPROM & Reboot. | - (Verify Reboot) | **YES** (NVS/Flash) |
| **`B`** | `00` | **Bootloader:** Enter DFU mode. | Device disappears/Re-enumerates | **NO** (ESP32 uses native boot) |
| **`B`** | `(not 00)` | **Reboot:** Soft Restart. | - (Verify Reboot) | **YES** |
| **`l`** | `00-02` | **LED:** Control Status LED (Off/On/Blink). | Visual Confirmation | **YES** (GPIO Mapping) |
| **`X`** | `RR` (Hex) | **Reporting Mode:** Configure Serial Output. | Current Setting or OK | **YES** (RX Engine) |

**`X` Command Bitmask (Critical for Tests):**
*   `X00`: Radio OFF (Default).
*   `X21`: Standard (Report Messages + RSSI).
*   *Bit 0:* Hex-Reporting ON.
*   *Bit 3:* Monitor Mode (Raw Data High/Low).
*   *Bit 4:* Timing Info (Debug).
*   *Bit 5:* Append RSSI.

## 2. Low-Level Hardware Tests (CC1101)
Verifies SPI communication and transceiver configuration.

| Command | Parameter | Function / Test Goal | Expected RX | Critical for ESP32 |
| :--- | :--- | :--- | :--- | :--- |
| **`C`** | `Reg` (Hex) | **Register Read:** e.g., `C35` (MARCSTATE), `C30` (PartNum). | Hex Value (e.g., `0D` for RX) | **YES** (SPI Check) |
| **`W`** | `Addr` `Val` | **EEPROM Write:** Configure settings. | - | **YES** (Config Persistence) |
| **`x`** | `00-09` | **PA Table:** Set Transmission Power. | - | **YES** |
| **`f`** | `s<data>` | **FastRF:** High-Speed Test Mode (Sniffer). | - | **NO** (Advanced) |

## 3. SlowRF Protocols (868 MHz Standard)
Core functionality of CULFW.

### FS20 (Switching/Dimming)
*   **TX:** `F` + `<Housecode(4)>` + `<Address(2)>` + `<Command(2)>` [+ `<Time(2)>`]
    *   *Example:* `F12340111` (House 1234, Dev 01, On)
*   **RX:** Starts with `F`. Format: `FHHHHAAKK` (Hex).

### FHT (Heating Control - 80b & 8v)
*   **TX:** `T` + `<Housecode(4)>` + `<Command(2)>` + `<Origin(2)>` + `<Arg(2)>`
    *   *Example:* `T1234017720`
*   **RX:** Starts with `T`. Format: `THHHHNNNNNN`.
*   **Special Tests:** `T01` (Read Own Housecode), `T02` (Read Buffer), `T11` (Check Timer).

### EM (Energy Sensors)
*   **TX (Emulation):** `M` + `<Type>` + `<Addr>` + `<Seq>` + `<Total>` ...
*   **RX:** Starts with `E`. Format: `Ettaacc111122223333`.

### Weather & Sensors (KS300 / S300TH / HMS / ESA)
*   **RX KS300:** Starts with `K` (Read backwards!).
*   **RX HMS:** Starts with `H` + 12 Hex Digits.
*   **RX ESA:** Starts with `S`.

## 4. Complex / Bidirectional Protocols

### AskSin (Homematic / BidCos)
*   **Config:** `Ar` (RX Enable), `Ax` (Disable).
*   **TX:** `As` + `<Hex String (Len, Cnt, Type, Src, Dst, Payload)>`.
*   **RX:** Starts with `A`.

### MORITZ (MAX!)
*   **Config:** `Zr` (RX Enable), `Zx` (Disable).
*   **TX:** `Zs` + `<Hex String>`.
*   **RX:** Starts with `Z`.

## 5. 433 MHz / Proprietary Protocols

### Intertechno (IT)
*   **Config:** `it<time>` (Pulse Duration, Default 420us), `isr<num>` (Repeats, Default 6).
*   **TX:** `is` + `<12-Tristate-Address/Data>` (0, 1, F).
*   **RX:** Often Raw or via `i`-Prefix (if implemented).

### Somfy RTS
*   **Config:** `Yt<time>` (Symbol Time ~1240us), `Yr<num>` (Repeats).
*   **TX:** `Ys` + `<Key><Cmd><0><RollingCode><Address>`.

### UNIRoll
*   **TX:** `U` + `<Group><Device><Cmd>`.

## 6. Raw & Debug Protocols

| Command | Format | Description |
| :--- | :--- | :--- |
| **Raw Send** | `G...` | Send raw bit sequences with defined timings. Format: `G<Sync><Len>...<HighTime><LowTime><Data>`. |
| **Native** | `N` | Native Mode (RFM12B Compatibility). `Nr<mode>` to enable. |
| **Monitor** | `X..` (Bit 3) | Outputs raw pulses as `r` (Rising) and `f` (Falling) or timing values. |

## 7. Test Vector Templates (JSON)

Use these structures for automated Python test scripts.

### Basic FS20 TX Test
```json
{
  "protocol": "FS20",
  "setup_commands": ["X21"],
  "tx_test": {
    "command": "F12340111",
    "description": "Send FS20 Switch Command",
    "expected_behavior": "Packet dispatched via CC1101"
  },
  "rx_test": {
    "trigger_method": "External Hardware or Loopback",
    "expected_pattern": "^F12340111.*",
    "timeout_ms": 1000
  }
}
```

### Intertechno TX Test
```json
{
  "protocol": "Intertechno",
  "setup_commands": ["it420", "isr6"],
  "tx_test": {
    "command": "is000000000FF0",
    "description": "Send Intertechno V1 Code",
    "expected_behavior": "GDO0 toggles in bit-banging mode"
  }
}
```

### System Health Check (ESP32 Specific)
```json
{
  "test_suite": "SystemHealth",
  "tests": [
    { "cmd": "V", "expect_regex": "^V 1\\..* a-culfw" },
    { "cmd": "C30", "expect_regex": "^C30 = 00.*" }, 
    { "cmd": "t", "expect_regex": "^[0-9A-F]+$" }
  ]
}
```
