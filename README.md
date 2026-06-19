# wireless-ts-controlpanel-transmitter

Firmware for the **transmitter** of the wireless handheld control panel.
It runs on an Arduino UNO (ATmega328P) and uses an nRF24L01+ radio to communicate
with the master receiver. The transmitter reads its keypad, shows status on a
graphic LCD, monitors its battery, and exchanges button/state data with the master.

## Features

- **2.4 GHz radio link** via nRF24L01+ (RF24 library) with auto-ack and retries
- **Button input** through a TM1638 module (TM16xx library)
- **Graphic LCD status screen** (U8glib) showing active unit, battery, link, and status
- **Battery monitoring** with a voltage divider on an analog pin
- **Error handling** for transmit failures and lost communication
- Optional **debug** and **test** build modes

## Hardware

### nRF24L01+ wiring

| Module | Arduino UNO | ESP32 | ESP8266 |
|--------|-------------|-------|---------|
| GND    | GND         | GND   | GND     |
| Vcc    | 3.3V (ext.) | 3.3V  | 3.3V    |
| CE     | D9          | D4    | D4      |
| CSN    | D10         | D5    | D8      |
| CLK    | D13         | D18   | D5      |
| MOSI   | D11         | D23   | D7      |
| MISO   | D12         | D19   | D6      |

The radio operates on `CHANNEL 42` (configurable via build flag), `RF24_250KBPS`,
and `RF24_PA_MAX`. The transmitter writes to the master address (`RX0`) and listens
on its own address (`TS1`).

### Battery sensing

Battery voltage is read on `A0` through a voltage divider. Charge is mapped between
`BATTERY_EMPTY_VOLTAGE` (10 V) and `BATTERY_FULL_VOLTAGE` (12 V).

> **Note:** `A0` is shared between the GLCD enable line and the battery input. With
> `TEST=0`, this is a hardware conflict that requires rewiring (see source comments
> in `src/header.h`).

## Project layout

```
src/
  main.cpp      Setup and main loop
  radio.cpp     nRF24L01+ setup, transmit, and read helpers
  buttons.cpp   TM1638 button scanning
  glcd.cpp      Graphic LCD status screen
  battery.cpp   Battery charge measurement
  error.cpp     Error flags and handling
  header.h      Pins, addresses, command/status enums, transaction_unit struct
lib/            Vendored libraries (RF24, TM16xx, U8glib)
test/           Unity tests and hardware mocks
```

## Building

This is a [PlatformIO](https://platformio.org/) project. Available environments:

| Environment  | Description |
|--------------|-------------|
| `uno`        | Production build for Arduino UNO (`DEBUG=0`, `TEST=0`) |
| `uno-debug`  | UNO build with serial debug and test mode (`DEBUG=1`, `TEST=1`) |
| `native`     | Host build for running unit tests with Unity |

Build and upload:

```sh
pio run -e uno -t upload
```

Run the unit tests on the host:

```sh
pio test -e native
```

## Communication protocol

Each message is a `transaction_unit` (3 bytes): a `command`, the `buttons` bitmask,
and the `active_unit`. Commands are defined in `src/header.h`:

| Command         | Meaning |
|-----------------|---------|
| `COMM_NOTHING`  | No operation |
| `COMM_START_TX` | Start transmission |
| `COMM_BUTTON`   | Button state update |
| `COMM_STOP_TX`  | Stop transmission |
| `COMM_DATA`     | Data from master (carries active unit) |
| `COMM_ACK`      | Acknowledgement |

In the main loop the transmitter sends button presses to the master, reads incoming
data to track the active unit, updates the battery/status, and refreshes the display.
