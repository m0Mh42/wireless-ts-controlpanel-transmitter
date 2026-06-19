#pragma once
#include <stdint.h>

extern uint32_t mock_tm1638_buttons;

class TM1638 {
public:
    TM1638(uint8_t dio, uint8_t clk, uint8_t stb,
           uint8_t digits, bool activate, uint8_t intensity) {}
    void begin(bool activate, uint8_t intensity) {}
    uint32_t getButtons() { return mock_tm1638_buttons; }
};
