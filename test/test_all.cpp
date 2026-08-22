#include <unity.h>
#include <cstring>

// ============================================================
// Include mock headers first so types are known
// ============================================================
#include "Arduino.h"
#include "SPI.h"
#include "TM1638.h"
#include "header.h"

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
bool     mock_rf24_listening;
uint8_t  mock_rf24_address_width;

#define MOCK_RF24_BUF_SIZE 64
uint8_t  mock_rf24_tx_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_tx_len;
uint8_t  mock_rf24_rx_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_rx_len;

// TM1638 mock state
uint32_t mock_tm1638_buttons;
char mock_tm1638_display_str[32];
bool mock_tm1638_cleared;

// Mock matrix button press
int mock_matrix_pressed_btn = -1;

// SPI instance
SPIClass SPI;

// ============================================================
// Arduino mock function definitions
// ============================================================
extern "C" {

int digitalRead(uint8_t pin)
{
    if (mock_matrix_pressed_btn >= 0) {
        int r = mock_matrix_pressed_btn / 3;
        int c = mock_matrix_pressed_btn % 3;
        const uint8_t row_pins[5] = {MATRIX_ROW_1, MATRIX_ROW_2, MATRIX_ROW_3, MATRIX_ROW_4, MATRIX_ROW_5};
        const uint8_t col_pins[3] = {MATRIX_COL_1, MATRIX_COL_2, MATRIX_COL_3};
        if (pin == col_pins[c]) {
            return (mock_pin_written[row_pins[r]] == LOW) ? LOW : HIGH;
        }
        return HIGH;
    }
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

void delayMicroseconds(unsigned int us) {}

int analogRead(uint8_t pin)
{
    return mock_analog_read_value;
}

}

// ============================================================
// Include sources under test
// ============================================================
#include "../src/header.cpp"
#include "../src/safety_crc32.cpp"
#include "../src/safety_protocol.cpp"
#include "../src/error.cpp"
#include "../src/radio.cpp"
#include "../src/buttons.cpp"
#include "../src/battery.cpp"
#include "../src/display.cpp"

// ============================================================
// Mock reset helper
// ============================================================
static void mock_reset(void)
{
    for (int i = 0; i < 20; i++) {
        mock_pin_read[i] = HIGH;
        mock_pin_written[i] = HIGH;
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
    mock_rf24_listening = false;
    mock_rf24_address_width = 0;
    mock_rf24_tx_len = 0;
    mock_rf24_rx_len = 0;
    memset(mock_rf24_tx_buf, 0, MOCK_RF24_BUF_SIZE);
    memset(mock_rf24_rx_buf, 0, MOCK_RF24_BUF_SIZE);

    mock_tm1638_buttons = 0;
    mock_tm1638_cleared = false;
    mock_tm1638_display_str[0] = '\0';
    mock_matrix_pressed_btn = -1;

    error_state = ERROR_NONE;
    error_counter = 0;
}

// ============================================================
// CRC-32 & SAFETY PROTOCOL TESTS
// ============================================================
static void test_crc32_self_test(void)
{
    TEST_ASSERT_TRUE(crc32_self_test());
}

static void test_crc32_standard_vector(void)
{
    const uint8_t vector[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    uint32_t crc = crc32_calculate_ieee(vector, sizeof(vector), 0xFFFFFFFFUL);
    TEST_ASSERT_EQUAL_HEX32(0xCBF43926UL, crc);
}

static void test_safety_frame_serialization_roundtrip(void)
{
    SafetyFrame tx;
    tx.preamble = PROTOCOL_MAGIC_PREAMBLE;
    tx.version = PROTOCOL_VERSION;
    tx.sender_id = NODE_ID_TRANSMITTER;
    tx.target_id = NODE_ID_RECEIVER;
    tx.sequence_number = 105;
    tx.session_id = DEFAULT_SESSION_ID;
    tx.frame_type = FrameType::COMMAND;
    tx.payload_length = sizeof(ApplicationPayload);

    SafetyProtocolEngine::pack_buttons(BUTTON_REGISTER_AVANTI | BUTTON_COLOR_ATTACCO, tx.payload);
    tx.payload.active_unit = 3;
    tx.payload.status_flags = (uint8_t)TS_STAT_ON;
    tx.payload.battery_mv = 11800;
    tx.payload.diagnostic_flags = 0x1234;
    tx.payload.spare1 = 0;
    tx.payload.spare2 = 0;

    uint8_t buf[SAFETY_FRAME_SIZE];
    bool ser_ok = SafetyProtocolEngine::serialize_frame(tx, buf, sizeof(buf));
    TEST_ASSERT_TRUE(ser_ok);

    SafetyFrame rx;
    SafetyValidationResult res = SafetyProtocolEngine::deserialize_and_validate(
        buf, sizeof(buf), NODE_ID_TRANSMITTER, NODE_ID_RECEIVER, DEFAULT_SESSION_ID, 100, 10, rx
    );

    TEST_ASSERT_EQUAL(SafetyValidationResult::VALID, res);
    TEST_ASSERT_EQUAL_UINT32(105, rx.sequence_number);
    TEST_ASSERT_EQUAL_HEX16(BUTTON_REGISTER_AVANTI | BUTTON_COLOR_ATTACCO, rx.payload.buttons_primary);
    TEST_ASSERT_EQUAL_HEX16((uint16_t)~(BUTTON_REGISTER_AVANTI | BUTTON_COLOR_ATTACCO), rx.payload.buttons_complement);
    TEST_ASSERT_EQUAL_UINT8(3, rx.payload.active_unit);
    TEST_ASSERT_EQUAL_UINT16(11800, rx.payload.battery_mv);
}

static void test_safety_frame_crc_corruption_rejected(void)
{
    SafetyFrame tx;
    tx.sender_id = NODE_ID_TRANSMITTER;
    tx.target_id = NODE_ID_RECEIVER;
    tx.sequence_number = 200;
    tx.session_id = DEFAULT_SESSION_ID;
    tx.frame_type = FrameType::COMMAND;
    SafetyProtocolEngine::pack_buttons(BUTTON_ANILOX_AV_ANTERIORE, tx.payload);

    uint8_t buf[SAFETY_FRAME_SIZE];
    SafetyProtocolEngine::serialize_frame(tx, buf, sizeof(buf));

    // Corrupt 1 single payload bit
    buf[14] ^= 0x01;

    SafetyFrame rx;
    SafetyValidationResult res = SafetyProtocolEngine::deserialize_and_validate(
        buf, sizeof(buf), NODE_ID_TRANSMITTER, NODE_ID_RECEIVER, DEFAULT_SESSION_ID, 199, 10, rx
    );

    TEST_ASSERT_EQUAL(SafetyValidationResult::ERR_CRC32_MISMATCH, res);
}

static void test_safety_frame_sequence_replay_rejected(void)
{
    SafetyFrame tx;
    tx.sender_id = NODE_ID_TRANSMITTER;
    tx.target_id = NODE_ID_RECEIVER;
    tx.sequence_number = 50; // Stale sequence (last was 100)
    tx.session_id = DEFAULT_SESSION_ID;
    tx.frame_type = FrameType::COMMAND;
    SafetyProtocolEngine::pack_buttons(0, tx.payload);

    uint8_t buf[SAFETY_FRAME_SIZE];
    SafetyProtocolEngine::serialize_frame(tx, buf, sizeof(buf));

    SafetyFrame rx;
    SafetyValidationResult res = SafetyProtocolEngine::deserialize_and_validate(
        buf, sizeof(buf), NODE_ID_TRANSMITTER, NODE_ID_RECEIVER, DEFAULT_SESSION_ID, 100, 10, rx
    );

    TEST_ASSERT_EQUAL(SafetyValidationResult::ERR_SEQUENCE_STALE_OR_REPLAY, res);
}

static void test_safety_frame_complement_bitflip_rejected(void)
{
    SafetyFrame tx;
    tx.sender_id = NODE_ID_TRANSMITTER;
    tx.target_id = NODE_ID_RECEIVER;
    tx.sequence_number = 101;
    tx.session_id = DEFAULT_SESSION_ID;
    tx.frame_type = FrameType::COMMAND;
    
    // Direct memory corruption: primary and complement don't match
    tx.payload.buttons_primary = 0x0004;
    tx.payload.buttons_complement = 0x0004; // Should be ~0x0004 = 0xFFFB

    uint8_t buf[SAFETY_FRAME_SIZE];
    SafetyProtocolEngine::serialize_frame(tx, buf, sizeof(buf));

    SafetyFrame rx;
    SafetyValidationResult res = SafetyProtocolEngine::deserialize_and_validate(
        buf, sizeof(buf), NODE_ID_TRANSMITTER, NODE_ID_RECEIVER, DEFAULT_SESSION_ID, 100, 10, rx
    );

    TEST_ASSERT_EQUAL(SafetyValidationResult::ERR_COMPLEMENTARY_BIT_FAULT, res);
}

// ============================================================
// HEADER TESTS (mapfloat, mapint)
// ============================================================
static void test_mapfloat_maps_correctly(void)
{
    float result = mapfloat(5.0f, 0, 10, 0, 100);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, result);
}

static void test_mapint_maps_correctly(void)
{
    int result = mapint(5.0f, 0, 10, 0, 100);
    TEST_ASSERT_EQUAL_INT(50, result);
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
    uint16_t mv = battery_millivolts();
    TEST_ASSERT_UINT16_WITHIN(100, 12000, mv);
}

static void test_battery_voltage_empty_formula(void)
{
    mock_reset();
    mock_analog_read_value = (int)(10.0 * VOLTAGE_DIVIDER_COEFFICIENT / 5.0 * 1024.0);
    uint8_t charge = battery_charge();
    TEST_ASSERT_EQUAL_UINT8(0, charge);
    uint16_t mv = battery_millivolts();
    TEST_ASSERT_UINT16_WITHIN(100, 10000, mv);
}

// ============================================================
// BUTTONS TESTS (15-button matrix)
// ============================================================
static void test_buttons_begin_initializes(void)
{
    mock_reset();
    buttons_begin();
    TEST_ASSERT_TRUE(true);
}

static void test_read_buttons_returns_zero_when_idle(void)
{
    mock_reset();
    for (int i = 0; i < 20; i++) mock_pin_read[i] = HIGH;
    uint16_t result = read_buttons();
    TEST_ASSERT_EQUAL_UINT16(0, result);
}

static void test_read_buttons_row0_col0(void)
{
    mock_reset();
    mock_matrix_pressed_btn = 0; // Row 0, Col 0 -> bit 0
    uint16_t result = read_buttons();
    TEST_ASSERT_EQUAL_HEX16(0x0001, result);
}

static void test_read_buttons_row4_col2(void)
{
    mock_reset();
    mock_matrix_pressed_btn = 14; // Row 4, Col 2 -> bit 14
    uint16_t result = read_buttons();
    TEST_ASSERT_EQUAL_HEX16(0x4000, result);
}

// ============================================================
// RADIO TESTS
// ============================================================
static void test_radio_setup_configures_safety(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);
    TEST_ASSERT_TRUE(mock_rf24_begin_called);
    TEST_ASSERT_EQUAL_UINT8(5, mock_rf24_address_width);
    TEST_ASSERT_EQUAL(RF24_PA_MAX, mock_rf24_pa_level);
    TEST_ASSERT_EQUAL(RF24_250KBPS, mock_rf24_data_rate);
    TEST_ASSERT_EQUAL(SAFETY_FRAME_SIZE, mock_rf24_payload_size);
    TEST_ASSERT_EQUAL(CHANNEL, mock_rf24_channel);
}

static void test_radio_send_safety_frame_success(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);

    bool sent = radio_send_safety_frame(&test_radio, 0x0001, 12000, TS_STAT_ON, 42);
    TEST_ASSERT_TRUE(sent);
    TEST_ASSERT_EQUAL(SAFETY_FRAME_SIZE, mock_rf24_tx_len);

    SafetyFrame rx;
    SafetyValidationResult res = SafetyProtocolEngine::deserialize_and_validate(
        mock_rf24_tx_buf, SAFETY_FRAME_SIZE, NODE_ID_TRANSMITTER, NODE_ID_RECEIVER, DEFAULT_SESSION_ID, 41, 10, rx
    );
    TEST_ASSERT_EQUAL(SafetyValidationResult::VALID, res);
    TEST_ASSERT_EQUAL_UINT32(42, rx.sequence_number);
    TEST_ASSERT_EQUAL_HEX16(0x0001, rx.payload.buttons_primary);
    TEST_ASSERT_EQUAL_UINT16(12000, rx.payload.battery_mv);
}

static void test_radio_channel_hopping(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);

    uint8_t initial_ch = radio_get_channel();
    radio_hop_next_channel(&test_radio);
    uint8_t next_ch = radio_get_channel();

    TEST_ASSERT_NOT_EQUAL(initial_ch, next_ch);
    TEST_ASSERT_EQUAL_UINT8(next_ch, mock_rf24_channel);
}

// ============================================================
// ERROR TESTS
// ============================================================
static void test_error_handling(void)
{
    mock_reset();
    TEST_ASSERT_EQUAL_INT(ERROR_NONE, error_state);
    error_set(ERROR_TRANSMIT_FAILED);
    TEST_ASSERT_TRUE(error_is_active(ERROR_TRANSMIT_FAILED));
    error_clear(ERROR_TRANSMIT_FAILED);
    TEST_ASSERT_FALSE(error_is_active(ERROR_TRANSMIT_FAILED));
}

// ============================================================
void setUp(void) {}
void tearDown(void) {}

int main(void)
{
    UNITY_BEGIN();

    // CRC & Safety Protocol tests
    RUN_TEST(test_crc32_self_test);
    RUN_TEST(test_crc32_standard_vector);
    RUN_TEST(test_safety_frame_serialization_roundtrip);
    RUN_TEST(test_safety_frame_crc_corruption_rejected);
    RUN_TEST(test_safety_frame_sequence_replay_rejected);
    RUN_TEST(test_safety_frame_complement_bitflip_rejected);

    // Header & Math tests
    RUN_TEST(test_mapfloat_maps_correctly);
    RUN_TEST(test_mapint_maps_correctly);
    RUN_TEST(test_mapint_battery_voltage_full);
    RUN_TEST(test_mapint_battery_voltage_empty);

    // Battery tests
    RUN_TEST(test_battery_charge_range);
    RUN_TEST(test_battery_voltage_full_formula);
    RUN_TEST(test_battery_voltage_empty_formula);

    // Buttons tests
    RUN_TEST(test_buttons_begin_initializes);
    RUN_TEST(test_read_buttons_returns_zero_when_idle);
    RUN_TEST(test_read_buttons_row0_col0);
    RUN_TEST(test_read_buttons_row4_col2);

    // Radio tests
    RUN_TEST(test_radio_setup_configures_safety);
    RUN_TEST(test_radio_send_safety_frame_success);
    RUN_TEST(test_radio_channel_hopping);

    // Error tests
    RUN_TEST(test_error_handling);

    return UNITY_END();
}
