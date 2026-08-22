# Hardware Verification & Pinout Notes

## 1. Pin Assignment Resolution (Clean Hardware Mapping)

In earlier prototype iterations, `A0` had a conflict when graphic LCD enable signals shared the analog pin. In the current architecture:

- **Battery ADC sensing** is exclusively routed to **`A0`** with a hardware RC filter (100 kΩ / 100 nF) and resistive divider ($R_1 = 100\text{ k}\Omega, R_2 = 40\text{ k}\Omega$, ratio $k = 0.2857$).
- **TM1638 Display** uses dedicated digital control lines on **`D8` (DIO)**, **`A4` (CLK)**, and **`A5` (STB)**.
- **Keypad Matrix (5x3)** occupies **`D2..D6` (Rows 1..5)** and **`D7, A1, A2` (Cols 1..3)**.
- **nRF24L01+ SPI** occupies **`D9` (CE)**, **`D10` (CSN)**, **`D11` (MOSI)**, **`D12` (MISO)**, and **`D13` (SCK)**.

All pin assignments are non-conflicting and fully verified.

---

## 2. Power Supply & Decoupling Requirements

The nRF24L01+ requires stable 3.3V DC power during transmit bursts (up to 15 mA peak current). 
- Always place a **10 µF electrolytic or tantalum capacitor** in parallel with a **100 nF ceramic capacitor** directly across the VCC and GND pins of the nRF24 module header.
- Do not power the nRF24 directly from the Arduino Uno's internal low-current 3.3V regulator without proper decoupling capacitance.
