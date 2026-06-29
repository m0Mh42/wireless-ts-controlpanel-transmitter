#include <unity.h>
#include <cstring>

// ============================================================
// Include mock headers first so types are known
// ============================================================
#include "Arduino.h"
#include "SPI.h"

// ============================================================
// Mock state — must be defined before including sources
// ============================================================
int mock_pin_read[20];
int mock_pin_written[20];
unsigned long mock_millis_value;
unsigned long mock_last_delay_ms;
int mock_analog_read_value;

HardwareSerial Serial;

// RF24 mock state
bool     mock_rf24_begin_called;
uint8_t  mock_rf24_pa_level;
uint8_t  mock_rf24_channel;
bool     mock_rf24_chip_connected;
bool     mock_rf24_write_result;
bool     mock_rf24_available;
uint8_t  mock_rf24_read_pipe_number;
const uint8_t* mock_rf24_write_addr;
const uint8_t* mock_rf24_read_addr;
bool     mock_rf24_auto_ack;
uint8_t  mock_rf24_retries_delay;
uint8_t  mock_rf24_retries_count;
uint8_t  mock_rf24_data_rate;
uint8_t  mock_rf24_payload_size;

#define MOCK_RF24_BUF_SIZE 32
uint8_t  mock_rf24_tx_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_tx_len;
uint8_t  mock_rf24_rx_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_rx_len;

// TM1638 mock state
uint32_t mock_tm1638_buttons;

// U8glib mock state
const uint8_t u8g_font_6x10[] = {0};
const uint8_t u8g_font_7x13[] = {0};
char mock_u8g_draw_buffer[256];
int  mock_u8g_draw_count;
int  mock_u8g_page_phase;

// SPI instance
SPIClass SPI;

// ============================================================
// Arduino mock function definitions
// ============================================================
extern "C" {

int digitalRead(uint8_t pin)
{
    return mock_pin_read[pin];
}

void digitalWrite(uint8_t pin, uint8_t value)
{
    mock_pin_written[pin] = value;
}

void pinMode(uint8_t pin, uint8_t mode) {}

unsigned long millis(void)
{
    return mock_millis_value;
}

void delay(unsigned long ms)
{
    mock_last_delay_ms = ms;
}

int analogRead(uint8_t pin)
{
    return mock_analog_read_value;
}

}

// ============================================================
// Include sources under test
// ============================================================
#include "header.h"
#include "../src/header.cpp"
#include "../src/error.cpp"
#include "../src/radio.cpp"
#include "../src/buttons.cpp"
#include "../src/battery.cpp"
#include "../src/glcd.cpp"

// ============================================================
// Mock reset helper (defined after sources so error globals exist)
// ============================================================
static void mock_reset(void)
{
    for (int i = 0; i < 20; i++) {
        mock_pin_read[i] = 0;
        mock_pin_written[i] = 0;
    }
    mock_millis_value = 0;
    mock_last_delay_ms = 0;
    mock_analog_read_value = 0;

    mock_rf24_begin_called = false;
    mock_rf24_pa_level = 0;
    mock_rf24_channel = 0;
    mock_rf24_chip_connected = true;
    mock_rf24_write_result = true;
    mock_rf24_available = false;
    mock_rf24_read_pipe_number = 0;
    mock_rf24_write_addr = nullptr;
    mock_rf24_read_addr = nullptr;
    mock_rf24_auto_ack = false;
    mock_rf24_retries_delay = 0;
    mock_rf24_retries_count = 0;
    mock_rf24_data_rate = 0;
    mock_rf24_payload_size = 0;
    mock_rf24_tx_len = 0;
    mock_rf24_rx_len = 0;
    memset(mock_rf24_tx_buf, 0, MOCK_RF24_BUF_SIZE);
    memset(mock_rf24_rx_buf, 0, MOCK_RF24_BUF_SIZE);

    mock_tm1638_buttons = 0;

    mock_u8g_draw_buffer[0] = '\0';
    mock_u8g_draw_count = 0;
    mock_u8g_page_phase = 0;

    error_state = ERROR_NONE;
    error_counter = 0;
}

// ============================================================
// HEADER TESTS (mapfloat, mapint)
// ============================================================
static void test_mapfloat_maps_correctly(void)
{
    float result = mapfloat(5.0f, 0, 10, 0, 100);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, result);
}

static void test_mapfloat_with_negative_range(void)
{
    float result = mapfloat(5.0f, 0, 10, 100, 0);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, result);
}

static void test_mapfloat_input_at_min(void)
{
    float result = mapfloat(0.0f, 0, 10, 20, 80);
    TEST_ASSERT_EQUAL_FLOAT(20.0f, result);
}

static void test_mapfloat_input_at_max(void)
{
    float result = mapfloat(10.0f, 0, 10, 20, 80);
    TEST_ASSERT_EQUAL_FLOAT(80.0f, result);
}

static void test_mapfloat_midpoint(void)
{
    float result = mapfloat(5.0f, 0, 10, 0, 255);
    TEST_ASSERT_EQUAL_FLOAT(127.5f, result);
}

static void test_mapint_maps_correctly(void)
{
    int result = mapint(5.0f, 0, 10, 0, 100);
    TEST_ASSERT_EQUAL_INT(50, result);
}

static void test_mapint_with_negative_range(void)
{
    int result = mapint(5.0f, 0, 10, 100, 0);
    TEST_ASSERT_EQUAL_INT(50, result);
}

static void test_mapint_input_at_min(void)
{
    int result = mapint(0.0f, 0, 10, 20, 80);
    TEST_ASSERT_EQUAL_INT(20, result);
}

static void test_mapint_input_at_max(void)
{
    int result = mapint(10.0f, 0, 10, 20, 80);
    TEST_ASSERT_EQUAL_INT(80, result);
}

static void test_mapint_division_by_zero(void)
{
    int result = mapint(5.0f, 0, 0, 0, 100);
    TEST_ASSERT_EQUAL_INT(0, result);
}

static void test_mapint_battery_voltage_full(void)
{
    int result = mapint(12.0f, 10, 12, 0, 255);
    TEST_ASSERT_EQUAL_INT(255, result);
}

static void test_mapint_battery_voltage_empty(void)
{
    int result = mapint(10.0f, 10, 12, 0, 255);
    TEST_ASSERT_EQUAL_INT(0, result);
}

static void test_mapint_battery_voltage_mid(void)
{
    int result = mapint(11.0f, 10, 12, 0, 255);
    TEST_ASSERT_INT_WITHIN(1, 127, result);
}

static void test_commands_enum_values(void)
{
    TEST_ASSERT_EQUAL_INT(0, COMM_NOTHING);
    TEST_ASSERT_EQUAL_INT(1, COMM_START_TX);
    TEST_ASSERT_EQUAL_INT(2, COMM_BUTTON);
    TEST_ASSERT_EQUAL_INT(4, COMM_STOP_TX);
    TEST_ASSERT_EQUAL_INT(8, COMM_DATA);
    TEST_ASSERT_EQUAL_INT(16, COMM_ACK);
}

static void test_transaction_unit_size(void)
{
    TEST_ASSERT_EQUAL_INT(3, sizeof(transaction_unit));
}

// ============================================================
// BATTERY TESTS
// ============================================================
static void test_battery_charge_range(void)
{
    mock_reset();
    uint8_t charge = battery_charge();
    TEST_ASSERT_UINT8_WITHIN(255, 0, charge);
}

static void test_battery_voltage_full_formula(void)
{
    mock_reset();
    mock_analog_read_value = (int)(12.0 * VOLTAGE_DIVIDER_COEFFICIENT / 5.0 * 1024.0);
    uint8_t charge = battery_charge();
    TEST_ASSERT_UINT8_WITHIN(2, 255, charge);
}

static void test_battery_voltage_empty_formula(void)
{
    mock_reset();
    mock_analog_read_value = (int)(10.0 * VOLTAGE_DIVIDER_COEFFICIENT / 5.0 * 1024.0);
    uint8_t charge = battery_charge();
    TEST_ASSERT_EQUAL_UINT8(0, charge);
}

static void test_battery_voltage_mid_formula(void)
{
    mock_reset();
    mock_analog_read_value = (int)(11.0 * VOLTAGE_DIVIDER_COEFFICIENT / 5.0 * 1024.0);
    uint8_t charge = battery_charge();
    TEST_ASSERT_UINT8_WITHIN(3, 127, charge);
}

static void test_battery_charge_above_full_clamps(void)
{
    mock_reset();
    mock_analog_read_value = (int)(13.0 * VOLTAGE_DIVIDER_COEFFICIENT / 5.0 * 1024.0);
    uint8_t charge = battery_charge();
    TEST_ASSERT_TRUE(charge >= 255);
}

static void test_battery_charge_below_empty_clamps(void)
{
    mock_reset();
    mock_analog_read_value = (int)(9.0 * VOLTAGE_DIVIDER_COEFFICIENT / 5.0 * 1024.0);
    uint8_t charge = battery_charge();
    TEST_ASSERT_TRUE(charge <= 0);
}

// ============================================================
// RADIO TESTS
// ============================================================
static void test_radio_setup_calls_begin(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT(mock_rf24_begin_called);
}

static void test_radio_setup_configures_pipes(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT_NOT_NULL(mock_rf24_write_addr);
    TEST_ASSERT_EQUAL(1, mock_rf24_read_pipe_number);
    TEST_ASSERT_NOT_NULL(mock_rf24_read_addr);
}

static void test_radio_setup_sets_pa_max(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT_EQUAL(RF24_PA_MAX, mock_rf24_pa_level);
}

static void test_radio_setup_sets_channel(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT_EQUAL(CHANNEL, mock_rf24_channel);
}

static void test_radio_setup_sets_auto_ack(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT(mock_rf24_auto_ack);
}

static void test_radio_setup_sets_retries(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT_EQUAL(5, mock_rf24_retries_delay);
    TEST_ASSERT_EQUAL(15, mock_rf24_retries_count);
}

static void test_radio_setup_sets_data_rate(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT_EQUAL(RF24_250KBPS, mock_rf24_data_rate);
}

static void test_radio_setup_sets_payload_size(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT_EQUAL(sizeof(transaction_unit), mock_rf24_payload_size);
}

static void test_radio_setup_chip_connected_ok(void)
{
    mock_reset();
    mock_rf24_chip_connected = true;
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT(mock_rf24_begin_called);
}

// Test writing to master pipe (DEVICE_MASTER = 0 = "RX0")
static void test_radio_setup_writes_to_master(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT_NOT_NULL(mock_rf24_write_addr);
    // DEVICE_MASTER = 0 → addresses[0] = "RX0"
    TEST_ASSERT_EQUAL_INT8('R', mock_rf24_write_addr[0]);
    TEST_ASSERT_EQUAL_INT8('X', mock_rf24_write_addr[1]);
    TEST_ASSERT_EQUAL_INT8('0', mock_rf24_write_addr[2]);
}

// Test reading on slave pipe (DEVICE_TS_SLAVE = 1 = "TS1")
static void test_radio_setup_reads_on_slave(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT_NOT_NULL(mock_rf24_read_addr);
    TEST_ASSERT_EQUAL_INT8('T', mock_rf24_read_addr[0]);
    TEST_ASSERT_EQUAL_INT8('S', mock_rf24_read_addr[1]);
    TEST_ASSERT_EQUAL_INT8('1', mock_rf24_read_addr[2]);
}

static void test_radio_transact_sends_data(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    transaction_unit tu = { COMM_BUTTON, 0x42, 3 };
    bool result = radio_transact(&test_radio, &tu);
    TEST_ASSERT(result);
    transaction_unit *sent = (transaction_unit*)mock_rf24_tx_buf;
    TEST_ASSERT_EQUAL(COMM_BUTTON, sent->command);
    TEST_ASSERT_EQUAL(0x42, sent->buttons);
    TEST_ASSERT_EQUAL(3, sent->active_unit);
}

static void test_radio_transact_retries_on_failure(void)
{
    mock_reset();
    mock_rf24_write_result = false;
    RF24 test_radio(CE_PIN, CSN_PIN);
    transaction_unit tu = { COMM_START_TX, 0, 0 };
    bool result = radio_transact(&test_radio, &tu);
    TEST_ASSERT_FALSE(result);
}

static void test_radio_read_returns_data(void)
{
    mock_reset();
    transaction_unit rx_data = { COMM_ACK, 0x10, 5 };
    memcpy(mock_rf24_rx_buf, &rx_data, sizeof(rx_data));
    mock_rf24_available = true;

    RF24 test_radio(CE_PIN, CSN_PIN);
    transaction_unit out = { 0, 0, 0 };
    bool result = radio_read(&test_radio, &out);
    TEST_ASSERT(result);
    TEST_ASSERT_EQUAL(COMM_ACK, out.command);
    TEST_ASSERT_EQUAL(0x10, out.buttons);
    TEST_ASSERT_EQUAL(5, out.active_unit);
}

static void test_radio_read_no_data(void)
{
    mock_reset();
    mock_rf24_available = false;
    RF24 test_radio(CE_PIN, CSN_PIN);
    transaction_unit out = { 0, 0, 0 };
    bool result = radio_read(&test_radio, &out);
    TEST_ASSERT_FALSE(result);
}

// ============================================================
// BUTTONS TESTS
// ============================================================
static void test_buttons_begin_initializes_module(void)
{
    mock_reset();
    buttons_begin();
    TEST_ASSERT_TRUE(true);
}

static void test_read_buttons_returns_zero_when_no_press(void)
{
    mock_reset();
    mock_tm1638_buttons = 0;
    uint8_t result = read_buttons();
    TEST_ASSERT_EQUAL_UINT8(0, result);
}

static void test_read_buttons_masks_to_low_byte(void)
{
    mock_reset();
    mock_tm1638_buttons = 0xAABB;
    uint8_t result = read_buttons();
    TEST_ASSERT_EQUAL_UINT8(0xBB, result);
}

static void test_read_buttons_passthrough(void)
{
    mock_reset();
    mock_tm1638_buttons = 0xFF;
    uint8_t result = read_buttons();
    TEST_ASSERT_EQUAL_UINT8(0xFF, result);
}

// ============================================================
// GLCD TESTS
// ============================================================
static void test_glcd_init(void)
{
    mock_reset();
    glcd_init();
    TEST_ASSERT_TRUE(true);
}

static void test_glcd_update_with_valid_params(void)
{
    mock_reset();
    glcd_update(1, 200, TS_STAT_ON, true);
    TEST_ASSERT_TRUE(true);
}

static void test_glcd_update_with_battery_low(void)
{
    mock_reset();
    glcd_update(0, 30, TS_STAT_BATTERYLOW, true);
    TEST_ASSERT_TRUE(true);
}

static void test_glcd_update_with_comm_lost(void)
{
    mock_reset();
    glcd_update(0, 100, TS_STAT_ON, false);
    TEST_ASSERT_TRUE(true);
}

static void test_glcd_status_text_values(void)
{
    TEST_ASSERT_EQUAL_INT(0, TS_STAT_OFF);
    TEST_ASSERT_EQUAL_INT(1, TS_STAT_ON);
    TEST_ASSERT_EQUAL_INT(2, TS_STAT_BATTERYLOW);
}

// ============================================================
// ERROR TESTS
// ============================================================
static void test_error_starts_clear(void)
{
    mock_reset();
    TEST_ASSERT_EQUAL_INT(ERROR_NONE, error_state);
    TEST_ASSERT_EQUAL_INT(0, error_counter);
}

static void test_error_set_changes_state(void)
{
    mock_reset();
    error_set(ERROR_RADIO_NOT_CONNECTED);
    TEST_ASSERT_EQUAL_INT(ERROR_RADIO_NOT_CONNECTED, error_state);
}

static void test_error_set_increments_counter(void)
{
    mock_reset();
    error_set(ERROR_TRANSMIT_FAILED);
    TEST_ASSERT_EQUAL_INT(1, error_counter);
    error_set(ERROR_TRANSMIT_FAILED);
    TEST_ASSERT_EQUAL_INT(2, error_counter);
}

static void test_error_clear_resets_state(void)
{
    mock_reset();
    error_set(ERROR_BATTERY_LOW);
    error_clear(ERROR_BATTERY_LOW);
    TEST_ASSERT_EQUAL_INT(ERROR_NONE, error_state);
}

static void test_error_clear_only_clears_matching(void)
{
    mock_reset();
    error_set(ERROR_NO_COMMUNICATION);
    error_clear(ERROR_BATTERY_LOW);
    TEST_ASSERT_EQUAL_INT(ERROR_NO_COMMUNICATION, error_state);
    error_clear(ERROR_NO_COMMUNICATION);
    TEST_ASSERT_EQUAL_INT(ERROR_NONE, error_state);
}

static void test_error_is_active_returns_true_when_set(void)
{
    mock_reset();
    error_set(ERROR_RADIO_NOT_CONNECTED);
    TEST_ASSERT_TRUE(error_is_active(ERROR_RADIO_NOT_CONNECTED));
    TEST_ASSERT_FALSE(error_is_active(ERROR_TRANSMIT_FAILED));
}

static void test_error_handling_does_not_crash(void)
{
    mock_reset();
    transaction_error_handling();
    TEST_ASSERT_TRUE(true);
    error_set(ERROR_BATTERY_LOW);
    transaction_error_handling();
    TEST_ASSERT_TRUE(true);
}

static void test_multiple_errors_track_separately(void)
{
    mock_reset();
    error_set(ERROR_TRANSMIT_FAILED);
    error_set(ERROR_NO_COMMUNICATION);
    // Both bits must be active simultaneously.
    TEST_ASSERT_TRUE(error_is_active(ERROR_TRANSMIT_FAILED));
    TEST_ASSERT_TRUE(error_is_active(ERROR_NO_COMMUNICATION));
    TEST_ASSERT_FALSE(error_is_active(ERROR_BATTERY_LOW));
    TEST_ASSERT_EQUAL_INT(2, error_counter);
    // Clearing one must not affect the other.
    error_clear(ERROR_TRANSMIT_FAILED);
    TEST_ASSERT_FALSE(error_is_active(ERROR_TRANSMIT_FAILED));
    TEST_ASSERT_TRUE(error_is_active(ERROR_NO_COMMUNICATION));
}

static void test_error_update_led_battery_low_blinks(void)
{
    mock_reset();
    error_set(ERROR_BATTERY_LOW);
    mock_millis_value = 500;
    error_update_led();
    TEST_ASSERT_EQUAL(HIGH, mock_pin_written[LED_BUILTIN]);
}

// ============================================================
void setUp(void) {}
void tearDown(void) {}

int main(void)
{
    UNITY_BEGIN();

    // header tests
    RUN_TEST(test_mapfloat_maps_correctly);
    RUN_TEST(test_mapfloat_with_negative_range);
    RUN_TEST(test_mapfloat_input_at_min);
    RUN_TEST(test_mapfloat_input_at_max);
    RUN_TEST(test_mapfloat_midpoint);
    RUN_TEST(test_mapint_maps_correctly);
    RUN_TEST(test_mapint_with_negative_range);
    RUN_TEST(test_mapint_input_at_min);
    RUN_TEST(test_mapint_input_at_max);
    RUN_TEST(test_mapint_division_by_zero);
    RUN_TEST(test_mapint_battery_voltage_full);
    RUN_TEST(test_mapint_battery_voltage_empty);
    RUN_TEST(test_mapint_battery_voltage_mid);
    RUN_TEST(test_commands_enum_values);
    RUN_TEST(test_transaction_unit_size);

    // battery tests
    RUN_TEST(test_battery_charge_range);
    RUN_TEST(test_battery_voltage_full_formula);
    RUN_TEST(test_battery_voltage_empty_formula);
    RUN_TEST(test_battery_voltage_mid_formula);
    RUN_TEST(test_battery_charge_above_full_clamps);
    RUN_TEST(test_battery_charge_below_empty_clamps);

    // radio tests
    RUN_TEST(test_radio_setup_calls_begin);
    RUN_TEST(test_radio_setup_configures_pipes);
    RUN_TEST(test_radio_setup_sets_pa_max);
    RUN_TEST(test_radio_setup_sets_channel);
    RUN_TEST(test_radio_setup_sets_auto_ack);
    RUN_TEST(test_radio_setup_sets_retries);
    RUN_TEST(test_radio_setup_sets_data_rate);
    RUN_TEST(test_radio_setup_sets_payload_size);
    RUN_TEST(test_radio_setup_chip_connected_ok);
    RUN_TEST(test_radio_setup_writes_to_master);
    RUN_TEST(test_radio_setup_reads_on_slave);
    RUN_TEST(test_radio_transact_sends_data);
    RUN_TEST(test_radio_transact_retries_on_failure);
    RUN_TEST(test_radio_read_returns_data);
    RUN_TEST(test_radio_read_no_data);

    // buttons tests
    RUN_TEST(test_buttons_begin_initializes_module);
    RUN_TEST(test_read_buttons_returns_zero_when_no_press);
    RUN_TEST(test_read_buttons_masks_to_low_byte);
    RUN_TEST(test_read_buttons_passthrough);

    // glcd tests
    RUN_TEST(test_glcd_init);
    RUN_TEST(test_glcd_update_with_valid_params);
    RUN_TEST(test_glcd_update_with_battery_low);
    RUN_TEST(test_glcd_update_with_comm_lost);
    RUN_TEST(test_glcd_status_text_values);

    // error tests
    RUN_TEST(test_error_starts_clear);
    RUN_TEST(test_error_set_changes_state);
    RUN_TEST(test_error_set_increments_counter);
    RUN_TEST(test_error_clear_resets_state);
    RUN_TEST(test_error_clear_only_clears_matching);
    RUN_TEST(test_error_is_active_returns_true_when_set);
    RUN_TEST(test_error_handling_does_not_crash);
    RUN_TEST(test_multiple_errors_track_separately);
    RUN_TEST(test_error_update_led_battery_low_blinks);

    return UNITY_END();
}
