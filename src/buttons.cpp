#include "buttons.h"

static TM1638 tm1638Buttons(BUTTONS_TM1638_DIO_PIN, BUTTONS_TM1638_CLK_PIN, BUTTONS_TM1638_STB_PIN, 8, false, 0);

void buttons_begin()
{
    tm1638Buttons.begin(false, 0);
}

uint8_t read_buttons()
{
    uint32_t rawButtons = tm1638Buttons.getButtons();
    return (uint8_t)(rawButtons & 0xFF);
}