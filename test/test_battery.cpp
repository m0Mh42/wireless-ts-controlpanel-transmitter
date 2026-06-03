#include <unity.h>
#include "battery.h"

void setUp(void) {}
void tearDown(void) {}

void test_battery_charge_range(void)
{
    uint8_t charge = battery_charge();
    TEST_ASSERT_UINT8_WITHIN(255, 0, charge);
}

void test_battery_voltage_full_formula(void)
{
    // Simulate 12V battery: analogRead of 12 * 0.3 / 5 * 1024
    // 12V * 0.3 = 3.6V at divider output
    // 3.6V / 5V * 1024 = 737 (approx)
    // mapint(12, 10, 12, 0, 255) = 255
    int adc_value = (int)(12.0 * VOLTAGE_DIVIDER_COEFFICIENT / 5.0 * 1024.0);
    TEST_ASSERT_TRUE(adc_value > 700);

    double battery_volt = adc_value / 1024.0 * 5 * VOLTAGE_DIVIDER_COEFFICIENT;
    int charge = mapint((float)battery_volt, BATTERY_EMPTY_VOLTAGE, BATTERY_FULL_VOLTAGE, 0, 255);
    TEST_ASSERT_EQUAL_INT(255, charge);
}

void test_battery_voltage_empty_formula(void)
{
    // Simulate 10V battery: analogRead of 10 * 0.3 / 5 * 1024
    // 10V * 0.3 = 3.0V at divider output
    // 3.0V / 5V * 1024 = 614 (approx)
    int adc_value = (int)(10.0 * VOLTAGE_DIVIDER_COEFFICIENT / 5.0 * 1024.0);
    TEST_ASSERT_TRUE(adc_value > 600);

    double battery_volt = adc_value / 1024.0 * 5 * VOLTAGE_DIVIDER_COEFFICIENT;
    int charge = mapint((float)battery_volt, BATTERY_EMPTY_VOLTAGE, BATTERY_FULL_VOLTAGE, 0, 255);
    TEST_ASSERT_EQUAL_INT(0, charge);
}

void test_battery_voltage_mid_formula(void)
{
    // Simulate 11V battery: analogRead of 11 * 0.3 / 5 * 1024
    int adc_value = (int)(11.0 * VOLTAGE_DIVIDER_COEFFICIENT / 5.0 * 1024.0);
    TEST_ASSERT_TRUE(adc_value > 650);

    double battery_volt = adc_value / 1024.0 * 5 * VOLTAGE_DIVIDER_COEFFICIENT;
    int charge = mapint((float)battery_volt, BATTERY_EMPTY_VOLTAGE, BATTERY_FULL_VOLTAGE, 0, 255);
    TEST_ASSERT_INT_WITHIN(1, 127, charge);
}

void test_battery_above_full_clamps(void)
{
    double battery_volt = 13.0;
    int charge = mapint((float)battery_volt, BATTERY_EMPTY_VOLTAGE, BATTERY_FULL_VOLTAGE, 0, 255);
    TEST_ASSERT_TRUE(charge >= 255);
}

void test_battery_below_empty_clamps(void)
{
    double battery_volt = 9.0;
    int charge = mapint((float)battery_volt, BATTERY_EMPTY_VOLTAGE, BATTERY_FULL_VOLTAGE, 0, 255);
    TEST_ASSERT_TRUE(charge <= 0);
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_battery_charge_range);
    RUN_TEST(test_battery_voltage_full_formula);
    RUN_TEST(test_battery_voltage_empty_formula);
    RUN_TEST(test_battery_voltage_mid_formula);
    RUN_TEST(test_battery_above_full_clamps);
    RUN_TEST(test_battery_below_empty_clamps);
    UNITY_END();
}

void loop() {}