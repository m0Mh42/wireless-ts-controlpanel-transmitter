#include <unity.h>
#include "radio.h"

static RF24 test_radio(CE_PIN, CSN_PIN);
static transaction_unit tx_unit;

void setUp(void)
{
    tx_unit.command = COMM_NOTHING;
    tx_unit.buttons = 0;
    tx_unit.active_unit = 0;
}

void tearDown(void) {}

void test_radio_init_sets_defaults(void)
{
    radio_setup(&test_radio);
    TEST_ASSERT_TRUE(true);
}

void test_radio_transact_with_null_pointer(void)
{
    bool result = radio_transact(&test_radio, NULL);
    TEST_ASSERT_FALSE(result);
}

void test_radio_read_with_no_data(void)
{
    bool result = radio_read(&test_radio, &tx_unit);
    TEST_ASSERT_FALSE(result);
}

void test_radio_transact_sends_correct_structure(void)
{
    tx_unit.command = COMM_BUTTON;
    tx_unit.buttons = 0xAB;
    tx_unit.active_unit = 1;
    radio_transact(&test_radio, &tx_unit);
    TEST_ASSERT_EQUAL_UINT8(COMM_BUTTON, tx_unit.command);
    TEST_ASSERT_EQUAL_UINT8(0xAB, tx_unit.buttons);
    TEST_ASSERT_EQUAL_UINT8(1, tx_unit.active_unit);
}

void test_radio_command_values(void)
{
    tx_unit.command = COMM_START_TX;
    TEST_ASSERT_EQUAL_UINT8(1, tx_unit.command);

    tx_unit.command = COMM_STOP_TX;
    TEST_ASSERT_EQUAL_UINT8(4, tx_unit.command);

    tx_unit.command = COMM_DATA;
    TEST_ASSERT_EQUAL_UINT8(8, tx_unit.command);

    tx_unit.command = COMM_ACK;
    TEST_ASSERT_EQUAL_UINT8(16, tx_unit.command);
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_radio_init_sets_defaults);
    RUN_TEST(test_radio_transact_with_null_pointer);
    RUN_TEST(test_radio_read_with_no_data);
    RUN_TEST(test_radio_transact_sends_correct_structure);
    RUN_TEST(test_radio_command_values);
    UNITY_END();
}

void loop() {}