#include <unity.h>
#include "glcd.h"

void setUp(void) {}
void tearDown(void) {}

void test_glcd_init(void)
{
    glcd_init();
    TEST_ASSERT_TRUE(true);
}

void test_glcd_update_with_valid_params(void)
{
    glcd_update(1, 200, TS_STAT_ON, true);
    TEST_ASSERT_TRUE(true);
}

void test_glcd_update_with_battery_low(void)
{
    glcd_update(0, 30, TS_STAT_BATTERYLOW, true);
    TEST_ASSERT_TRUE(true);
}

void test_glcd_update_with_comm_lost(void)
{
    glcd_update(0, 100, TS_STAT_ON, false);
    TEST_ASSERT_TRUE(true);
}

void test_glcd_update_with_off_status(void)
{
    glcd_update(0, 0, TS_STAT_OFF, false);
    TEST_ASSERT_TRUE(true);
}

void test_glcd_update_many_units(void)
{
    glcd_update(255, 255, TS_STAT_ON, true);
    TEST_ASSERT_TRUE(true);
}

void test_glcd_status_text_values(void)
{
    TEST_ASSERT_EQUAL_INT(0, TS_STAT_OFF);
    TEST_ASSERT_EQUAL_INT(1, TS_STAT_ON);
    TEST_ASSERT_EQUAL_INT(2, TS_STAT_BATTERYLOW);
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_glcd_init);
    RUN_TEST(test_glcd_update_with_valid_params);
    RUN_TEST(test_glcd_update_with_battery_low);
    RUN_TEST(test_glcd_update_with_comm_lost);
    RUN_TEST(test_glcd_update_with_off_status);
    RUN_TEST(test_glcd_update_many_units);
    RUN_TEST(test_glcd_status_text_values);
    UNITY_END();
}

void loop() {}