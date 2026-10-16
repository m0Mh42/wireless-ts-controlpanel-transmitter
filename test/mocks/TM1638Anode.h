#pragma once
#include "TM1638.h"

class TM1638Anode : public TM1638 {
public:
    TM1638Anode(uint8_t dio, uint8_t clk, uint8_t stb,
                bool activate = true, uint8_t intensity = 7)
        : TM1638(dio, clk, stb, 10, activate, intensity) {}
};
