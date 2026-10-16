#ifndef _H_BATTERY
#define _H_BATTERY 1

#include "header.h"

// Initialize Battery ADC pin (with pullup enabled)
void battery_init();

// Battery Charge is a value between 0 and 255
uint8_t battery_charge();

// Battery Voltage in millivolts (e.g. 11500 for 11.50V)
uint16_t battery_millivolts();

// Returns true if a real 12V battery is connected (>= 8000 mV), false if floating / bench power (< 8000 mV)
bool battery_is_connected();

#endif
