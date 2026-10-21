#ifndef _H_HEADER
#define _H_HEADER 1

#include <Arduino.h>
#include "protocol.h"

// Default RF Power Level: RF24_PA_MAX for maximum distance across industrial halls
#ifndef RF24_PA_LEVEL_DEFAULT
#define RF24_PA_LEVEL_DEFAULT RF24_PA_MAX
#endif

// Default On-Air Data Rate: RF24_1MBPS for standard robust turnaround, or RF24_250KBPS for maximum sensitivity
#ifndef RF24_DATARATE_DEFAULT
#define RF24_DATARATE_DEFAULT RF24_1MBPS
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
#define CSN_PIN 10
#define CE_PIN A0
#elif defined(ESP32)
#define CSN_PIN 5
#define CE_PIN 4
#elif defined(ESP8266)
#define CSN_PIN D8
#define CE_PIN D4
#else
#define CSN_PIN 10
#define CE_PIN A0
#endif

#ifndef RF24_SPI_SPEED
#define RF24_SPI_SPEED 2000000UL // 2 MHz SPI clock
#endif

// Guard Channel outside standard Wi-Fi
#ifndef CHANNEL
#define CHANNEL 78
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef TEST
#define TEST 0
#endif

// Safety Watchdog & Cycle Timings
#define SAFETY_WATCHDOG_TIMEOUT_MS 150UL
#define TX_CYCLE_PERIOD_MS 20UL
#define TELEMETRY_WAIT_TIMEOUT_MS 25UL // Headroom for clone chips, receiver display refresh, and PA/LNA settling

// RF24 5-Byte Pipe Addresses (Pipes 1..5 share top 4 bytes per Nordic MultiCeiver spec)
static const uint8_t RF24_PIPE_CONTROL[5] = {'S', 'A', 'F', 'E', '1'};   // Transmitter -> Receiver
static const uint8_t RF24_PIPE_TELEMETRY[5] = {'S', 'A', 'F', 'E', '2'}; // Receiver -> Transmitter
static const uint8_t RF24_SAFETY_PIPE[5] = {'S', 'A', 'F', 'E', '1'};    // Legacy alias

// Battery Voltage Input (Transmitter.NET: R1.2/R2.2 -> U2.A3, 10k + 10k divider)
#define BATTERY_INPUT_PIN A3
#define VOLTAGE_DIVIDER_COEFFICIENT 0.5f
#define BATTERY_FULL_VOLTAGE 4.2f
#define BATTERY_EMPTY_VOLTAGE 3.0f

// 4x4 Button Matrix Pins (Transmitter.NET Connector P4: D5..D9 and A1, A4, A5)
#define MATRIX_ROW_1 9  // D9
#define MATRIX_ROW_2 8  // D18
#define MATRIX_ROW_3 7  // D19
#define MATRIX_ROW_4 A1 // D20
#define MATRIX_COL_1 5  // D3
#define MATRIX_COL_2 A5 // D4
#define MATRIX_COL_3 6  // D5
#define MATRIX_COL_4 A4 // D6

// Button bitmasks for 4x4 matrix (16-bit uint16_t: 15 buttons + ESTOP)
typedef enum buttons_enum
{
    // Register (4 buttons)
    BUTTON_REGISTER_AVANTI = 0x0001,     // D9 - D3
    BUTTON_REGISTER_INDIETRO = 0x0002,   // D9 - D4
    BUTTON_REGISTER_ABTERIORE = 0x0004,  // D9 - D5
    BUTTON_REGISTER_POSTERIORE = 0x0008, // D9 - D6

    // Anilox (4 buttons)
    BUTTON_ANILOX_AV_ANTERIORE = 0x0010,  // D18 - D3
    BUTTON_ANILOX_AV_POSTERIORE = 0x0020, // D18 - D4
    BUTTON_ANILOX_IN_ANTERIORE = 0x0040,  // D18 - D5
    BUTTON_ANILOX_IN_POSTERIORE = 0x0080, // D18 - D6

    // Cliche (4 buttons)
    BUTTON_CLICHE_AV_ANTERIORE = 0x0100,  // D19 - D3
    BUTTON_CLICHE_IN_ANTERIORE = 0x0200,  // D19 - D4
    BUTTON_CLICHE_AV_POSTERIORE = 0x0400, // D19 - D5
    BUTTON_CLICHE_IN_POSTERIORE = 0x0800, // D19 - D6

    // Color (3 buttons)
    BUTTON_COLOR_ATTACCO = 0x1000, // D20 - D3
    BUTTON_COLOR_STACCO = 0x2000,  // D20 - D4
    BUTTON_COLOR_COLORE = 0x4000,  // D20 - D5

    // Emergency / Safety stop bit
    BUTTON_ESTOP_ACTIVE = 0x8000 // D20 - D6
} buttons_enum;

typedef enum ts_status
{
    TS_STAT_OFF = 0,
    TS_STAT_ON = 1,
    TS_STAT_BATTERYLOW = 2,
    TS_STAT_FAULT = 3
} ts_status;

float mapfloat(float x, long in_min, long in_max, long out_min, long out_max);
int mapint(float x, long in_min, long in_max, long out_min, long out_max);

#endif // _H_HEADER
