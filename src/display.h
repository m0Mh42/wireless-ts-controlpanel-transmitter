#ifndef _H_DISPLAY
#define _H_DISPLAY 1

#include <Arduino.h>
#include "header.h"

// TM1638 display IC pins (Transmitter.NET: P1.2->D2, P1.3->D3, P1.4->D4)
#define DISPLAY_TM1638_DIO_PIN 2
#define DISPLAY_TM1638_CLK_PIN 3
#define DISPLAY_TM1638_STB_PIN 4

// Indicator bitmask constants for horizontal status bar L (5 blocks: Green, Red, Yellow, Yellow, Yellow)
#define DISPLAY_IND_GREEN 0x01 // Block 1 (Green) - Comm Link OK
#define DISPLAY_IND_RED 0x02   // Block 2 (Red)   - Comm Link Fault
#define DISPLAY_IND_YEL1 0x04  // Block 3 (Yellow)
#define DISPLAY_IND_YEL2 0x08  // Block 4 (Yellow)
#define DISPLAY_IND_YEL3 0x10  // Block 5 (Yellow)

struct RadioTxDiagnostics;

void display_init();
void display_show_linking(uint8_t seconds_left);
void display_show_linking_debug(uint8_t channel, uint8_t error_code);
void display_update(uint8_t active_unit, uint8_t battery_level, ts_status status, bool comm_ok, uint16_t buttons = 0, uint16_t battery_mv = 0, uint8_t error_flags = 0);
void display_show_packet_debug(const RadioTxDiagnostics &diag, uint8_t channel, uint8_t lqi, bool chip_connected);
void display_show_radio_error();

void display_clear();
void display_set_brightness(uint8_t brightness);
void display_set_top(const char *str);
void display_set_bottom(const char *str);
void display_set_battery_bar(uint8_t bars);
void display_set_indicators(uint8_t mask);
void display_refresh();
void display_test_pattern();

// Access internal RAM buffer (for unit tests / validation)
const uint8_t *display_get_ram();

#endif
