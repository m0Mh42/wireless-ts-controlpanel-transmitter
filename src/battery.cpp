#include "battery.h"

uint8_t battery_charge()
{
    int battery = analogRead(BATTERY_INPUT_PIN);
    double battery_volt = battery / 1024.0 * 5 * VOLTAGE_DIVIDER_COEFFICIENT;
    return mapint(battery_volt, BATTERY_EMPTY_VOLTAGE, BATTERY_FULL_VOLTAGE, 0, 255);
}