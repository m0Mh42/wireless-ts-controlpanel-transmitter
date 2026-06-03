#ifndef _H_HEADER
#define _H_HEADER 1

#include <Arduino.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
#define CSN_PIN 10
#define CE_PIN 9
#elif defined(ESP32)
#define CSN_PIN 5
#define CE_PIN 4
#elif defined(ESP8266)
#define CSN_PIN D8
#define CE_PIN D4
#else
#error "Make Config yourself"
#endif

#define CHANNEL 42

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef TEST
#define TEST 0
#endif

// Battery Voltage Input
// NOTE: A0 is shared with GLCD_COMM_EN. When TEST=1, GLCD is not used so A0 is free for battery.
// When TEST=0, GLCD and battery share A0 - this is a hardware conflict that requires rewiring.
// See KNOWN_ISSUES.md for details on resolving this.
#define BATTERY_INPUT_PIN A0
#define VOLTAGE_DIVIDER_COEFFICIENT 0.3
#define BATTERY_FULL_VOLTAGE 12
#define BATTERY_EMPTY_VOLTAGE 10

// Device Addresses Enumeration
enum device_addresses_enum
{
    DEVICE_MASTER = 0,
    DEVICE_TS_SLAVE = 1
};

// Device Addresses
const uint8_t addresses[][4] = {"RX0", "TS1"};

// Transaction Unit Command Byte
typedef enum commands_enum
{
    COMM_NOTHING = 0,
    COMM_START_TX = 1,
    COMM_BUTTON = 2,
    COMM_STOP_TX = 4,
    COMM_DATA = 8,
    COMM_ACK = 16
} commands_enum;

typedef enum ts_status
{
    TS_STAT_OFF = 0,
    TS_STAT_ON = 1,
    TS_STAT_BATTERYLOW = 2
} ts_status;

// Transaction Unit
typedef struct transaction_unit
{
    uint8_t command;
    uint8_t buttons;
    uint8_t active_unit;
} transaction_unit;

float mapfloat(float x, long in_min, long in_max, long out_min, long out_max);
int mapint(float x, long in_min, long in_max, long out_min, long out_max);

#endif
