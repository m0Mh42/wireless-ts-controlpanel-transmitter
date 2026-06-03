#include <unity.h>
#include "error.h"

void setUp(void)
{
    error_state = ERROR_NONE;
    error_counter = 0;
}

void tearDown(void) {}

void test_error_starts_clear(void)
{
    TEST_ASSERT_EQUAL_INT(ERROR_NONE, error_state);
    TEST_ASSERT_EQUAL_INT(0, error_counter);
}

void test_error_set_changes_state(void)
{
    error_set(ERROR_RADIO_NOT_CONNECTED);
    TEST_ASSERT_EQUAL_INT(ERROR_RADIO_NOT_CONNECTED, error_state);
}

void test_error_set_increments_counter(void)
{
    error_set(ERROR_TRANSMIT_FAILED);
    TEST_ASSERT_EQUAL_INT(1, error_counter);

    error_set(ERROR_TRANSMIT_FAILED);
    TEST_ASSERT_EQUAL_INT(2, error_counter);
}

void test_error_clear_resets_state(void)
{
    error_set(ERROR_BATTERY_LOW);
    TEST_ASSERT_EQUAL_INT(ERROR_BATTERY_LOW, error_state);

    error_clear(ERROR_BATTERY_LOW);
    TEST_ASSERT_EQUAL_INT(ERROR_NONE, error_state);
}

void test_error_clear_only_clears_matching(void)
{
    error_set(ERROR_NO_COMMUNICATION);
    error_clear(ERROR_BATTERY_LOW);
    TEST_ASSERT_EQUAL_INT(ERROR_NO_COMMUNICATION, error_state);

    error_clear(ERROR_NO_COMMUNICATION);
    TEST_ASSERT_EQUAL_INT(ERROR_NONE, error_state);
}

void test_error_is_active_returns_true_when_set(void)
{
    error_set(ERROR_RADIO_NOT_CONNECTED);
    TEST_ASSERT_TRUE(error_is_active(ERROR_RADIO_NOT_CONNECTED));
    TEST_ASSERT_FALSE(error_is_active(ERROR_TRANSMIT_FAILED));
}

void test_error_handling_does_not_crash(void)
{
    transaction_error_handling();
    TEST_ASSERT_TRUE(true);

    error_set(ERROR_BATTERY_LOW);
    transaction_error_handling();
    TEST_ASSERT_TRUE(true);
}

void test_multiple_errors_track_separately(void)
{
    error_set(ERROR_TRANSMIT_FAILED);
    error_set(ERROR_NO_COMMUNICATION);

    TEST_ASSERT_EQUAL_INT(ERROR_NO_COMMUNICATION, error_state);
    TEST_ASSERT_EQUAL_INT(2, error_counter);
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_error_starts_clear);
    RUN_TEST(test_error_set_changes_state);
    RUN_TEST(test_error_set_increments_counter);
    RUN_TEST(test_error_clear_resets_state);
    RUN_TEST(test_error_clear_only_clears_matching);
    RUN_TEST(test_error_is_active_returns_true_when_set);
    RUN_TEST(test_error_handling_does_not_crash);
    RUN_TEST(test_multiple_errors_track_separately);
    UNITY_END();
}

void loop() {}