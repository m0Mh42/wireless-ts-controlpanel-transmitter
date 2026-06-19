#pragma once
#include <stdint.h>
#include <cstring>

// Font data placeholders
extern const uint8_t u8g_font_6x10[];
extern const uint8_t u8g_font_7x13[];

// Mock draw commands buffer for verification
extern char mock_u8g_draw_buffer[256];
extern int  mock_u8g_draw_count;
extern int  mock_u8g_page_phase;

class U8GLIB_KS0108_128 {
public:
    U8GLIB_KS0108_128(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                       uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
                       uint8_t en, uint8_t cs1, uint8_t cs2, uint8_t di) {}
    void begin() {}
    void setFont(const uint8_t*) {}
    void drawStr(uint8_t x, uint8_t y, const char* s) {
        (void)x; (void)y;
        strcpy(mock_u8g_draw_buffer, s);
        mock_u8g_draw_count++;
    }
    void drawFrame(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {}
    void drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {}
    void firstPage() { mock_u8g_page_phase = 0; }
    bool nextPage() {
        mock_u8g_page_phase++;
        return mock_u8g_page_phase == 1;
    }
};
