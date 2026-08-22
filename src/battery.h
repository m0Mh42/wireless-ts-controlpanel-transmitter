#ifndef _H_BATTERY
#define _H_BATTERY 1

#include "header.h"

// Battery Charge is a value between 0 and 255
uint8_t battery_charge();

// Battery Voltage in millivolts (e.g. 11500 for 11.50V)
uint16_t battery_millivolts();

#endif
