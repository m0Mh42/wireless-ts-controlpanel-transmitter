#include "battery.h"

void battery_init()
{
    pinMode(BATTERY_INPUT_PIN, INPUT_PULLUP);
}

uint16_t battery_millivolts()
{
    int battery = analogRead(BATTERY_INPUT_PIN);
    // (ADC / 1024.0) * 5.0 / VOLTAGE_DIVIDER_COEFFICIENT * 1000.0
    float mv = (battery / 1024.0f) * 5.0f * 1000.0f / VOLTAGE_DIVIDER_COEFFICIENT;
    if (mv < 0.0f)
        return 0;
    if (mv > 10000.0f)
        return 10000;
    return (uint16_t)mv;
}

bool battery_is_connected()
{
    // A single 18650 cell operates between 3.0V (empty cutoff) and 4.2V (fully charged).
    // An unconnected / floating pin or bench power via 5V/USB reads < 2500 mV.
    return (battery_millivolts() >= 2500);
}

uint8_t battery_charge()
{
    if (!battery_is_connected())
    {
        // Unconnected / floating pin: return 100% capacity (255) to suppress false low-battery alarms
        return 255;
    }
    uint16_t mv = battery_millivolts();
    int charge = mapint(mv, (long)(BATTERY_EMPTY_VOLTAGE * 1000.0f), (long)(BATTERY_FULL_VOLTAGE * 1000.0f), 0, 255);
    if (charge < 0)
        return 0;
    if (charge > 255)
        return 255;
    return (uint8_t)charge;
}