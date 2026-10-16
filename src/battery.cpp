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
    if (mv < 0.0f) return 0;
    if (mv > 20000.0f) return 20000;
    return (uint16_t)mv;
}

bool battery_is_connected()
{
    // In an industrial 12V system, an active battery operates between 10.0V and 14.4V.
    // An unconnected / floating pin or bench power via 5V/USB reads < 8000 mV.
    return (battery_millivolts() >= 8000);
}

uint8_t battery_charge()
{
    if (!battery_is_connected())
    {
        // Unconnected / floating pin: return 100% capacity (255) to suppress false low-battery alarms
        return 255;
    }
    int battery = analogRead(BATTERY_INPUT_PIN);
    float battery_volt = (battery / 1024.0f) * 5.0f / VOLTAGE_DIVIDER_COEFFICIENT;
    int charge = mapint(battery_volt, (long)BATTERY_EMPTY_VOLTAGE, (long)BATTERY_FULL_VOLTAGE, 0, 255);
    if (charge < 0) return 0;
    if (charge > 255) return 255;
    return (uint8_t)charge;
}