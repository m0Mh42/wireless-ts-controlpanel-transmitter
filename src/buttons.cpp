#include "buttons.h"
#include "header.h"

uint8_t read_buttons()
{
    uint8_t buttons = 0;
    buttons |= digitalRead(P1);
    buttons |= (digitalRead(P2) << 1);
    buttons |= (digitalRead(P3) << 2);
    buttons |= (digitalRead(P4) << 3);
    buttons |= (digitalRead(P5) << 4);
    buttons |= (digitalRead(P6) << 5);
    buttons |= (digitalRead(P7) << 6);
    buttons |= (digitalRead(P8) << 7);
    return buttons;
}