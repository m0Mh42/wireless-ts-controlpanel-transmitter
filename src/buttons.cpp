#include "buttons.h"

static const uint8_t row_pins[4] = {MATRIX_ROW_1, MATRIX_ROW_2, MATRIX_ROW_3, MATRIX_ROW_4};
static const uint8_t col_pins[4] = {MATRIX_COL_1, MATRIX_COL_2, MATRIX_COL_3, MATRIX_COL_4};

void buttons_begin()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    for (int i = 0; i < 4; i++)
    {
        pinMode(row_pins[i], OUTPUT);
        digitalWrite(row_pins[i], HIGH);
    }
    for (int i = 0; i < 4; i++)
    {
        pinMode(col_pins[i], INPUT_PULLUP);
    }
}

uint16_t read_buttons()
{
    uint16_t mask = 0;

    for (int r = 0; r < 4; r++)
    {
        digitalWrite(row_pins[r], LOW);
        delayMicroseconds(10);

        for (int c = 0; c < 4; c++)
        {
            if (digitalRead(col_pins[c]) == LOW)
            {
                int btn_id = (r * 4) + c;
                mask |= ((uint16_t)1 << btn_id);
            }
        }

        digitalWrite(row_pins[r], HIGH);
    }

    return mask;
}