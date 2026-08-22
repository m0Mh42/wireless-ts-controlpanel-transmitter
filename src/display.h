#ifndef _H_DISPLAY
#define _H_DISPLAY 1

#include <Arduino.h>
#include "header.h"

// TM1638 display IC pins
#define DISPLAY_TM1638_DIO_PIN 8
#define DISPLAY_TM1638_CLK_PIN A4
#define DISPLAY_TM1638_STB_PIN A5

void display_init();
void display_update(uint8_t active_unit, uint8_t battery_level, ts_status status, bool comm_ok);

#endif
