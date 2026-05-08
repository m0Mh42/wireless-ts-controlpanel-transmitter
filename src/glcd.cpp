#include "glcd.h"

static U8GLIB_KS0108_128 u8g(8, 9, 10, 11, 4, 5, 6, 7, 18, 14, 15, 17, 16); // 8Bit Com: D0..D7: 8,9,10,11,4,5,6,7 en=18, cs1=14, cs2=15,di=17,rw=16

static uint8_t current_active_unit = 0;
static uint8_t current_battery_level = 0;
static ts_status current_status = TS_STAT_OFF;
static bool current_comm_ok = false;

static const char *status_text(ts_status status)
{
    switch (status)
    {
    case TS_STAT_ON:
        return "STATUS: ON";
    case TS_STAT_BATTERYLOW:
        return "BATTERY LOW";
    case TS_STAT_OFF:
    default:
        return "STATUS: OFF";
    }
}

static const char *comm_text(bool comm_ok)
{
    return comm_ok ? "COMM: OK" : "COMM: LOST";
}

static const char *active_unit_text(uint8_t active_unit)
{
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "UNIT: %d", active_unit);
    return buffer;
}

static void draw_frame()
{
    uint8_t battery_percent = mapint(current_battery_level, 0, 255, 0, 100);
    uint8_t battery_width = battery_percent * 76 / 100;

    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(0, 2, "UTeco Transmitter");

    u8g.setFont(u8g_font_7x13);
    u8g.drawStr(0, 16, active_unit_text(current_active_unit));
    u8g.drawStr(0, 32, status_text(current_status));
    u8g.drawStr(0, 48, comm_text(current_comm_ok));

    u8g.drawFrame(0, 54, 80, 10);
    if (battery_width > 0)
    {
        u8g.drawBox(1, 55, battery_width, 8);
    }

    char battery_label[16];
    snprintf(battery_label, sizeof(battery_label), "%u%%", battery_percent);
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(86, 58, battery_label);
}

void glcd_init()
{
    u8g.begin();
    u8g.setFont(u8g_font_6x10);
}

void glcd_update(uint8_t active_unit, uint8_t battery_level, ts_status status, bool comm_ok)
{
    current_active_unit = active_unit;
    current_battery_level = battery_level;
    current_status = status;
    current_comm_ok = comm_ok;

    u8g.firstPage();
    do
    {
        draw_frame();
    } while (u8g.nextPage());
}
