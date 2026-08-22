#include "buttons.h"

static const uint8_t row_pins[5] = {MATRIX_ROW_1, MATRIX_ROW_2, MATRIX_ROW_3, MATRIX_ROW_4, MATRIX_ROW_5};
static const uint8_t col_pins[3] = {MATRIX_COL_1, MATRIX_COL_2, MATRIX_COL_3};

void buttons_begin()
{
    for (int i = 0; i < 5; i++) {
        pinMode(row_pins[i], OUTPUT);
        digitalWrite(row_pins[i], HIGH);
    }
    for (int i = 0; i < 3; i++) {
        pinMode(col_pins[i], INPUT_PULLUP);
    }
}

uint16_t read_buttons()
{
    uint16_t mask = 0;
    
    for (int r = 0; r < 5; r++) {
        digitalWrite(row_pins[r], LOW);
        delayMicroseconds(10); 
        
        for (int c = 0; c < 3; c++) {
            if (digitalRead(col_pins[c]) == LOW) {
                int btn_id = (r * 3) + c;
                mask |= (1 << btn_id);
            }
        }
        
        digitalWrite(row_pins[r], HIGH);
    }
    
    return mask;
}