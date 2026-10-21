#ifndef _H_BATTERY
#define _H_BATTERY 1

#include "header.h"

// Initialize Battery ADC pin (with pullup enabled)
void battery_init();

// Battery Charge is a value between 0 and 255
uint8_t battery_charge();

// Battery Voltage in millivolts (e.g. 3700 for 3.70V)
uint16_t battery_millivolts();

// Returns true if a real 18650 battery cell is connected (>= 2500 mV), false if floating / bench power (< 2500 mV)
bool battery_is_connected();

#endif
