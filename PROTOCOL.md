# UTECO Wireless Control Panel — Black Channel Safety Communication Protocol

**Standards Compliance**: EN 50159 | IEC 61784-3 | ISO 13849-1 (Category 3/4) | IEC 62061

The wireless link operates on the **EN 50159 / IEC 61784-3 Black Channel Principle**: no safety integrity is credited to the nRF24L01+ hardware or the 2.4 GHz RF medium. All message authentication, sequencing, dual-rail validation, and CRC checks are performed in the application Safety Communication Layer (SCL).

---

## 1. Frame Structure (32 Bytes Wire Format)

Every packet utilizes 100% of the 32-byte nRF24 hardware FIFO width:

```
Byte Offset | Field Name            | Type      | Description
------------------------------------------------------------------------------------------------------
0x00        | PREAMBLE              | uint8_t   | Magic sync byte: 0xA5
0x01        | VERSION               | uint8_t   | Protocol version: 0x01
0x02..0x03  | SENDER_NODE_ID        | uint16_t  | Transmitter: 0x0002, Receiver: 0x0001 (Little-Endian)
0x04..0x05  | TARGET_NODE_ID        | uint16_t  | Receiver: 0x0001, Transmitter: 0x0002 (Little-Endian)
0x06..0x09  | SEQUENCE_NUMBER       | uint32_t  | Strictly monotonic packet counter (Little-Endian)
0x0A..0x0B  | SESSION_TOKEN         | uint16_t  | Session identifier: 0x1234 (Little-Endian)
0x0C        | FRAME_TYPE            | uint8_t   | 0x10=HEARTBEAT, 0x20=COMMAND, 0x30=ESTOP, 0x40=TELEMETRY
0x0D        | PAYLOAD_LENGTH        | uint8_t   | Fixed 14 bytes (0x0E)
0x0E..0x0F  | BUTTONS_PRIMARY       | uint16_t  | 15-button matrix bitmask (Register, Anilox, Cliche, Color)
0x10..0x11  | BUTTONS_COMPLEMENT    | uint16_t  | Bitwise complement (~BUTTONS_PRIMARY) for RAM fault detection
0x12        | ACTIVE_UNIT           | uint8_t   | Printing Unit ID (0-15 BCD from PLC)
0x13        | STATUS_FLAGS          | uint8_t   | TS_STAT_ON (1), TS_STAT_BATTERYLOW (2), TS_STAT_OFF (0)
0x14..0x15  | BATTERY_MV            | uint16_t  | Battery millivolts (e.g. 11800 mV = 11.80 V)
0x16..0x17  | DIAGNOSTIC_FLAGS      | uint16_t  | Low byte: RF Channel, High byte: Link Quality Index %
0x18..0x19  | SPARE_1               | uint16_t  | Alignment padding / expansion
0x1A..0x1B  | SPARE_2               | uint16_t  | Alignment padding / expansion
0x1C..0x1F  | FRAME_CRC32           | uint32_t  | IEEE 802.3 CRC-32 over bytes 0x00..0x1B (28 bytes, Little-Endian)
------------------------------------------------------------------------------------------------------
Total Size: 32 Bytes
```

---

## 2. Industrial Wi-Fi Guard Channel Hopping

To prevent interference from standard IEEE 802.11b/g/n (20 MHz channels 1, 6, 11), the radio link uses inter-band guard channels:

- **Channel 04 (2404 MHz)**: Below Wi-Fi Channel 1
- **Channel 28 (2428 MHz)**: Guard band between Wi-Fi Channel 1 and 6
- **Channel 54 (2454 MHz)**: Guard band between Wi-Fi Channel 6 and 11
- **Channel 78 (2478 MHz)**: Upper guard band above Wi-Fi Channel 11 (Default Rendezvous)
- **Channel 82 (2482 MHz)**: Upper ISM edge

### Frequency Agility Rules:
1. **Transmitter Agility**: After 5 consecutive transmission timeouts (`MAX_CONSECUTIVE_TX_FAILS`), the transmitter increments to the next channel in the guard channel table.
2. **Receiver Agility**: If no valid safety frame is received for > 500 ms, the receiver cycles to the next guard channel to re-synchronize.

---

## 3. Safety Watchdog & Failsafe Behavior

- **Cyclic Period**: Transmitter transmits safety frames every **20 ms** (50 Hz).
- **Safety Watchdog Timeout**: Receiver enforces a strict **100 ms** watchdog timeout.
- **Failsafe Action**:
  - If no valid frame arrives within 100 ms or if CRC-32/sequence/complement integrity checks fail, the Receiver **instantly clears all button outputs** (`PLCO_REGISTER`, `PLCO_ANILOX`, `PLCO_CLICHE`, `PLCO_COLOR` driven LOW to 0).
  - All movements stop immediately with zero risk of stuck machine motion.
  - `PLCO_FASTSTOP` is maintained unasserted during communication dropout, but trips immediately if an explicit `BUTTON_ESTOP_ACTIVE` or `FrameType::ESTOP_ACTIVE` packet is received.
