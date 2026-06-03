#ifndef _H_BUTTONS
#define _H_BUTTONS

#include "header.h"
#include <TM1638.h>

// TM1638 button IC pins
#define BUTTONS_TM1638_DIO_PIN 8
#define BUTTONS_TM1638_CLK_PIN A4
#define BUTTONS_TM1638_STB_PIN A5

void buttons_begin();
uint8_t read_buttons();

#endif