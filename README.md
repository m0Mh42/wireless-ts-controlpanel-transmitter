# UTECO Wireless Control Panel — Transmitter Firmware

Industrial safety firmware for the **handheld pendant transmitter** of the UTECO flexographic printing press wireless control system. It runs on an ATmega328P (Arduino Uno) and implements the **EN 50159 / IEC 61784-3 Black Channel Safety Protocol** over an nRF24L01+ 2.4 GHz transceiver.

---

## Safety Architecture (EN 50159 / IEC 61784-3)

- **Black Channel Layer**: Strict message authentication, sequencing, CRC checking, and dual-rail validation in application code.
- **Deterministic Transmission Loop**: 20 ms cyclic period (50 Hz rate).
- **IEEE 802.3 CRC-32**: Cryptographic 32-bit polynomial (`0xEDB88320`) guarding every 32-byte frame.
- **Flash ROM CRC Optimization**: Table stored in `PROGMEM` (`pgm_read_dword`), conserving **1,024 bytes (50%) of RAM** on the ATmega328P.
- **Dual-Rail Encoding**: 16-bit button mask (`buttons_primary`) paired with bitwise inverted `buttons_complement` (`~buttons_primary`).
- **Monotonic Sequencing & Resync**: Sequence numbers increment monotonically; automatic resynchronization allows immediate reconnection whenever powered on in any sequence.
- **High-Power RF Transmission**: Default `RF24_PA_MAX` (+20 dBm on PA+LNA modules) for maximum coverage across plant distances.
- **Pinned Guard Channel**: Fixed on Channel 78 (2478 MHz) in the upper Wi-Fi guard band.

---

## Hardware Pinout & Wiring (ATmega328P / Arduino Uno)

### 1. nRF24L01+ Radio Interface
| Signal | Arduino Uno Pin | Description |
| :--- | :--- | :--- |
| **VCC** | 3.3V (Ext. Filtered) | 3.3V DC Power with 10µF + 100nF decoupling |
| **GND** | GND | Common Ground |
| **CE** | D9 | Chip Enable |
| **CSN** | D10 | SPI Chip Select Not |
| **SCK** | D13 | SPI Clock |
| **MOSI** | D11 | Master Out Slave In |
| **MISO** | D12 | Master In Slave Out |

### 2. 15-Button Matrix Keypad (5 Rows x 3 Columns)
| Signal | Pin | Mode | Description |
| :--- | :--- | :--- | :--- |
| `MATRIX_ROW_1` | D2 | Output (Driven LOW during scan) | Keypad Row 1 (Buttons 0..2) |
| `MATRIX_ROW_2` | D3 | Output (Driven LOW during scan) | Keypad Row 2 (Buttons 3..5) |
| `MATRIX_ROW_3` | D4 | Output (Driven LOW during scan) | Keypad Row 3 (Buttons 6..8) |
| `MATRIX_ROW_4` | D5 | Output (Driven LOW during scan) | Keypad Row 4 (Buttons 9..11) |
| `MATRIX_ROW_5` | D6 | Output (Driven LOW during scan) | Keypad Row 5 (Buttons 12..14) |
| `MATRIX_COL_1` | D7 | Input Pullup | Keypad Column 1 |
| `MATRIX_COL_2` | A1 (Pin 15) | Input Pullup | Keypad Column 2 |
| `MATRIX_COL_3` | A2 (Pin 16) | Input Pullup | Keypad Column 3 |

### 3. TM1638 7-Segment Display Module
| Signal | Pin | Description |
| :--- | :--- | :--- |
| `DISPLAY_TM1638_DIO_PIN` | D8 | Data Input/Output |
| `DISPLAY_TM1638_CLK_PIN` | A4 (Pin 18) | Clock Line |
| `DISPLAY_TM1638_STB_PIN` | A5 (Pin 19) | Strobe Line |

### 4. Battery Voltage Sensing
| Signal | Pin | Description |
| :--- | :--- | :--- |
| `BATTERY_INPUT_PIN` | A0 (Pin 14) | Analog ADC input via resistive voltage divider (Ratio = 0.2857) |

---

## 15-Button Matrix Mapping

| Button Index | Bitmask Constant | Hex Mask | Function |
| :--- | :--- | :--- | :--- |
| **0** | `BUTTON_REGISTER_AVANTI` | `0x0001` | Register Advance |
| **1** | `BUTTON_REGISTER_INDIETRO` | `0x0002` | Register Retract |
| **2** | `BUTTON_REGISTER_ABTERIORE` | `0x0004` | Register Anterior / Offset |
| **3** | `BUTTON_REGISTER_POSTERIORE` | `0x0008` | Register Posterior |
| **4** | `BUTTON_ANILOX_AV_ANTERIORE` | `0x0010` | Anilox Advance Anterior |
| **5** | `BUTTON_ANILOX_IN_ANTERIORE` | `0x0020` | Anilox Retract Anterior |
| **6** | `BUTTON_ANILOX_AV_POSTERIORE` | `0x0040` | Anilox Advance Posterior |
| **7** | `BUTTON_ANILOX_IN_POSTERIORE` | `0x0080` | Anilox Retract Posterior |
| **8** | `BUTTON_CLICHE_AV_ANTERIORE` | `0x0100` | Cliche Advance Anterior |
| **9** | `BUTTON_CLICHE_IN_ANTERIORE` | `0x0200` | Cliche Retract Anterior |
| **10** | `BUTTON_CLICHE_AV_POSTERIORE` | `0x0400` | Cliche Advance Posterior |
| **11** | `BUTTON_CLICHE_IN_POSTERIORE` | `0x0800` | Cliche Retract Posterior |
| **12** | `BUTTON_COLOR_ATTACCO` | `0x1000` | Color Engage (Attacco) |
| **13** | `BUTTON_COLOR_STACCO` | `0x2000` | Color Disengage (Stacco) |
| **14** | `BUTTON_COLOR_COLORE` | `0x4000` | Color Wash / Pump |

---

## Build, Upload & Test

```sh
# Run native unit tests on host (35/35 tests)
pio test -e native

# Compile production firmware for Arduino Uno (ACK payload mode)
pio run -e uno

# Compile production firmware for Arduino Uno (Ping-Pong mode)
pio run -e uno-pingpong

# Upload to Arduino Uno
pio run -e uno -t upload
```

### Memory Footprint (ATmega328P)
- **RAM**: 411 bytes / 2048 bytes (**20.1% utilized**, 1.63 KB free)
- **Flash ROM**: 12,084 bytes / 30,720 bytes (**39.3% utilized**)
