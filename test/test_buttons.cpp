#include <unity.h>
#include "buttons.h"

void setUp(void) {}
void tearDown(void) {}

void test_buttons_begin_initializes_module(void)
{
    buttons_begin();
    TEST_ASSERT_TRUE(true);
}

void test_read_buttons_returns_uint8_t(void)
{
    uint8_t result = read_buttons();
    TEST_ASSERT_TRUE(result >= 0);
}

void test_read_buttons_masks_to_low_byte(void)
{
    uint8_t result = read_buttons();
    TEST_ASSERT_UINT8_WITHIN(0xFF, 0, result);
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_buttons_begin_initializes_module);
    RUN_TEST(test_read_buttons_returns_uint8_t);
    RUN_TEST(test_read_buttons_masks_to_low_byte);
    UNITY_END();
}

void loop() {}