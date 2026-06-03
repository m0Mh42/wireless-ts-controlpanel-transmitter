#include <unity.h>
#include "header.h"

void setUp(void) {}
void tearDown(void) {}

void test_mapfloat_maps_correctly(void)
{
    float result = mapfloat(5.0f, 0, 10, 0, 100);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, result);
}

void test_mapfloat_with_negative_range(void)
{
    float result = mapfloat(5.0f, 0, 10, 100, 0);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, result);
}

void test_mapfloat_input_at_min(void)
{
    float result = mapfloat(0.0f, 0, 10, 20, 80);
    TEST_ASSERT_EQUAL_FLOAT(20.0f, result);
}

void test_mapfloat_input_at_max(void)
{
    float result = mapfloat(10.0f, 0, 10, 20, 80);
    TEST_ASSERT_EQUAL_FLOAT(80.0f, result);
}

void test_mapfloat_midpoint(void)
{
    float result = mapfloat(5.0f, 0, 10, 0, 255);
    TEST_ASSERT_EQUAL_FLOAT(127.5f, result);
}

void test_mapint_maps_correctly(void)
{
    int result = mapint(5.0f, 0, 10, 0, 100);
    TEST_ASSERT_EQUAL_INT(50, result);
}

void test_mapint_with_negative_range(void)
{
    int result = mapint(5.0f, 0, 10, 100, 0);
    TEST_ASSERT_EQUAL_INT(50, result);
}

void test_mapint_input_at_min(void)
{
    int result = mapint(0.0f, 0, 10, 20, 80);
    TEST_ASSERT_EQUAL_INT(20, result);
}

void test_mapint_input_at_max(void)
{
    int result = mapint(10.0f, 0, 10, 20, 80);
    TEST_ASSERT_EQUAL_INT(80, result);
}

void test_mapint_battery_voltage_full(void)
{
    // 12V battery → should map to 255
    int result = mapint(12.0f, 10, 12, 0, 255);
    TEST_ASSERT_EQUAL_INT(255, result);
}

void test_mapint_battery_voltage_empty(void)
{
    // 10V battery → should map to 0
    int result = mapint(10.0f, 10, 12, 0, 255);
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_mapint_battery_voltage_mid(void)
{
    // 11V battery → should map to ~127
    int result = mapint(11.0f, 10, 12, 0, 255);
    TEST_ASSERT_INT_WITHIN(1, 127, result);
}

void test_mapint_division_by_zero(void)
{
    int result = mapint(5.0f, 0, 0, 0, 100);
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_commands_enum_values(void)
{
    TEST_ASSERT_EQUAL_INT(0, COMM_NOTHING);
    TEST_ASSERT_EQUAL_INT(1, COMM_START_TX);
    TEST_ASSERT_EQUAL_INT(2, COMM_BUTTON);
    TEST_ASSERT_EQUAL_INT(4, COMM_STOP_TX);
    TEST_ASSERT_EQUAL_INT(8, COMM_DATA);
    TEST_ASSERT_EQUAL_INT(16, COMM_ACK);
}

void test_transaction_unit_size(void)
{
    TEST_ASSERT_EQUAL_INT(3, sizeof(transaction_unit));
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_mapfloat_maps_correctly);
    RUN_TEST(test_mapfloat_with_negative_range);
    RUN_TEST(test_mapfloat_input_at_min);
    RUN_TEST(test_mapfloat_input_at_max);
    RUN_TEST(test_mapfloat_midpoint);
    RUN_TEST(test_mapint_maps_correctly);
    RUN_TEST(test_mapint_with_negative_range);
    RUN_TEST(test_mapint_input_at_min);
    RUN_TEST(test_mapint_input_at_max);
    RUN_TEST(test_mapint_battery_voltage_full);
    RUN_TEST(test_mapint_battery_voltage_empty);
    RUN_TEST(test_mapint_battery_voltage_mid);
    RUN_TEST(test_mapint_division_by_zero);
    RUN_TEST(test_commands_enum_values);
    RUN_TEST(test_transaction_unit_size);
    UNITY_END();
}

void loop() {}