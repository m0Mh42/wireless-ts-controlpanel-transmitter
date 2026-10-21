#include <unity.h>
#include <cstring>

// ============================================================
// Include mock headers first so types are known
// ============================================================
#include "Arduino.h"
#include "SPI.h"
#include "TM1638.h"
#include "header.h"
#include "protocol.h"

// ============================================================
// Mock state — must be defined before including sources
// ============================================================
int mock_pin_read[20];
int mock_pin_written[20];
int mock_pin_mode[20];
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
bool     mock_rf24_ack_payload_enabled;
bool     mock_rf24_dynamic_payloads_enabled;
bool     mock_rf24_ack_payload_available;
bool     mock_rf24_auto_reply_telemetry = true;
uint8_t  mock_rf24_simulated_active_unit = 0;

#define MOCK_RF24_BUF_SIZE 64
uint8_t  mock_rf24_tx_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_tx_len;
uint8_t  mock_rf24_rx_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_rx_len;
uint8_t  mock_rf24_ack_payload_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_ack_payload_len;

// TM1638 mock state
uint32_t mock_tm1638_buttons;
char mock_tm1638_display_str[32];
bool mock_tm1638_cleared;

// Mock matrix button press
int mock_matrix_pressed_btn = -1;
uint16_t mock_matrix_pressed_mask = 0;

// SPI instance
SPIClass SPI;

// ============================================================
// Arduino mock function definitions
// ============================================================
extern "C" {

int digitalRead(uint8_t pin)
{
    if (mock_matrix_pressed_mask != 0) {
        const uint8_t row_pins[4] = {MATRIX_ROW_1, MATRIX_ROW_2, MATRIX_ROW_3, MATRIX_ROW_4};
        const uint8_t col_pins[4] = {MATRIX_COL_1, MATRIX_COL_2, MATRIX_COL_3, MATRIX_COL_4};
        for (int r = 0; r < 4; r++) {
            if (mock_pin_written[row_pins[r]] == LOW) {
                for (int c = 0; c < 4; c++) {
                    if (pin == col_pins[c]) {
                        int btn_id = (r * 4) + c;
                        if (mock_matrix_pressed_mask & ((uint16_t)1 << btn_id)) {
                            return LOW;
                        }
                    }
                }
            }
        }
        return HIGH;
    }
    if (mock_matrix_pressed_btn >= 0) {
        int r = mock_matrix_pressed_btn / 4;
        int c = mock_matrix_pressed_btn % 4;
        const uint8_t row_pins[4] = {MATRIX_ROW_1, MATRIX_ROW_2, MATRIX_ROW_3, MATRIX_ROW_4};
        const uint8_t col_pins[4] = {MATRIX_COL_1, MATRIX_COL_2, MATRIX_COL_3, MATRIX_COL_4};
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

void pinMode(uint8_t pin, uint8_t mode)
{
    if (pin < 20) mock_pin_mode[pin] = mode;
}

unsigned long millis(void)
{
    return mock_millis_value;
}

unsigned long micros(void)
{
    return mock_millis_value * 1000UL;
}

void delay(unsigned long ms)
{
    mock_last_delay_ms = ms;
    mock_millis_value += ms;
}

void delayMicroseconds(unsigned int us)
{
    (void)us;
    mock_millis_value += 1;
}

int analogRead(uint8_t pin)
{
    return mock_analog_read_value;
}

}

// ============================================================
// Include sources under test
// ============================================================
#include "../src/header.cpp"
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
        mock_pin_mode[i] = 0;
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
    mock_rf24_ack_payload_enabled = false;
    mock_rf24_dynamic_payloads_enabled = false;
    mock_rf24_ack_payload_available = false;
    mock_rf24_auto_reply_telemetry = true;
    mock_rf24_simulated_active_unit = 0;
    mock_rf24_tx_len = 0;
    mock_rf24_rx_len = 0;
    mock_rf24_ack_payload_len = 0;
    memset(mock_rf24_tx_buf, 0, MOCK_RF24_BUF_SIZE);
    memset(mock_rf24_rx_buf, 0, MOCK_RF24_BUF_SIZE);
    memset(mock_rf24_ack_payload_buf, 0, MOCK_RF24_BUF_SIZE);

    mock_tm1638_buttons = 0;
    mock_tm1638_cleared = false;
    mock_tm1638_display_str[0] = '\0';
    mock_matrix_pressed_btn = -1;
    mock_matrix_pressed_mask = 0;

    error_state = ERROR_NONE;
    error_counter = 0;
}

// ============================================================
// PROTOCOL TESTS
// ============================================================
static void test_protocol_packet_sizes(void)
{
    TEST_ASSERT_EQUAL_UINT32(8, sizeof(ControlPacket));
    TEST_ASSERT_EQUAL_UINT32(4, sizeof(TelemetryPacket));
}

static void test_protocol_verify_buttons_valid(void)
{
    uint16_t b = BUTTON_REGISTER_AVANTI | BUTTON_COLOR_ATTACCO;
    uint16_t b_inv = (uint16_t)~b;
    TEST_ASSERT_TRUE(protocol_verify_buttons(b, b_inv));
}

static void test_protocol_verify_buttons_corrupted(void)
{
    uint16_t b = BUTTON_REGISTER_AVANTI;
    uint16_t b_inv = (uint16_t)b; // Inverted bits don't match
    TEST_ASSERT_FALSE(protocol_verify_buttons(b, b_inv));
}

static void test_protocol_control_packet_construction(void)
{
    ControlPacket pkt;
    pkt.buttons = BUTTON_ANILOX_AV_ANTERIORE;
    pkt.buttons_inv = (uint16_t)~BUTTON_ANILOX_AV_ANTERIORE;
    pkt.battery_mv = 11800;
    pkt.status = TS_STAT_ON;
    pkt.seq = 105;

    TEST_ASSERT_EQUAL_HEX16(BUTTON_ANILOX_AV_ANTERIORE, pkt.buttons);
    TEST_ASSERT_TRUE(protocol_verify_buttons(pkt.buttons, pkt.buttons_inv));
    TEST_ASSERT_EQUAL_UINT16(11800, pkt.battery_mv);
    TEST_ASSERT_EQUAL_UINT8(TS_STAT_ON, pkt.status);
    TEST_ASSERT_EQUAL_UINT8(105, pkt.seq);
}

// ============================================================
// HEADER TESTS (mapfloat, mapint)
// ============================================================
static void test_mapfloat_maps_correctly(void)
{
    float val = mapfloat(50.0f, 0, 100, 0, 200);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, val);

    val = mapfloat(0.0f, 0, 100, -100, 100);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -100.0f, val);

    val = mapfloat(100.0f, 0, 100, -100, 100);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, val);
}

static void test_mapint_maps_correctly(void)
{
    int val = mapint(50.0f, 0, 100, 0, 200);
    TEST_ASSERT_EQUAL_INT(100, val);

    val = mapint(0.0f, 0, 100, -100, 100);
    TEST_ASSERT_EQUAL_INT(-100, val);

    val = mapint(100.0f, 0, 100, -100, 100);
    TEST_ASSERT_EQUAL_INT(100, val);
}

static void test_mapint_battery_voltage_full(void)
{
    float full_mv = 4200.0f;
    int charge = mapint(full_mv, 3000, 4200, 0, 255);
    TEST_ASSERT_EQUAL_INT(255, charge);
}

static void test_mapint_battery_voltage_empty(void)
{
    float empty_mv = 3000.0f;
    int charge = mapint(empty_mv, 3000, 4200, 0, 255);
    TEST_ASSERT_EQUAL_INT(0, charge);
}

// ============================================================
// BATTERY TESTS
// ============================================================
static void test_battery_init_sets_pullup(void)
{
    mock_reset();
    battery_init();
    TEST_ASSERT_EQUAL_INT(INPUT_PULLUP, mock_pin_mode[BATTERY_INPUT_PIN]);
}

static void test_battery_charge_range(void)
{
    mock_reset();
    battery_init();

    // Connected full 18650 battery (4.2V, ADC ~430): charge is 255
    mock_analog_read_value = 430;
    uint8_t c = battery_charge();
    TEST_ASSERT_UINT8_WITHIN(2, 255, c);

    // Connected empty 18650 battery (3.0V, ADC ~307): charge is 0
    mock_analog_read_value = 307;
    c = battery_charge();
    TEST_ASSERT_EQUAL_UINT8(0, c);
}

static void test_battery_voltage_full_formula(void)
{
    mock_reset();
    battery_init();

    // ADC = 430: (430 * 5.0) / (1024 * 0.5) = 4.199 V -> approx 4200 mV
    mock_analog_read_value = 430;
    uint16_t mv = battery_millivolts();
    TEST_ASSERT_UINT16_WITHIN(50, 4200, mv);
}

static void test_battery_voltage_empty_formula(void)
{
    mock_reset();
    battery_init();

    // ADC = 307: (307 * 5.0) / (1024 * 0.5) = 2.998 V -> approx 3000 mV
    mock_analog_read_value = 307;
    uint16_t mv = battery_millivolts();
    TEST_ASSERT_UINT16_WITHIN(50, 3000, mv);
}

static void test_battery_floating_pin_unconnected(void)
{
    mock_reset();
    battery_init();

    // Low voltage below 2.5V threshold -> not connected
    mock_analog_read_value = 150;
    TEST_ASSERT_FALSE(battery_is_connected());

    // Normal connected 18650 battery above 2.5V threshold -> connected
    mock_analog_read_value = 400;
    TEST_ASSERT_TRUE(battery_is_connected());
}

// ============================================================
// BUTTONS TESTS (4x4 Matrix)
// ============================================================
static void test_buttons_begin_initializes(void)
{
    mock_reset();
    buttons_begin();

    const uint8_t row_pins[4] = {MATRIX_ROW_1, MATRIX_ROW_2, MATRIX_ROW_3, MATRIX_ROW_4};
    const uint8_t col_pins[4] = {MATRIX_COL_1, MATRIX_COL_2, MATRIX_COL_3, MATRIX_COL_4};

    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_INT(OUTPUT, mock_pin_mode[row_pins[i]]);
        TEST_ASSERT_EQUAL_INT(HIGH, mock_pin_written[row_pins[i]]);
    }
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_INT(INPUT_PULLUP, mock_pin_mode[col_pins[i]]);
    }
}

static void test_read_buttons_returns_zero_when_idle(void)
{
    mock_reset();
    mock_matrix_pressed_btn = -1;
    uint16_t result = read_buttons();
    TEST_ASSERT_EQUAL_HEX16(0x0000, result);
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
    mock_matrix_pressed_btn = 14; // Row 3, Col 2 -> bit 14 (BUTTON_COLOR_COLORE)
    uint16_t result = read_buttons();
    TEST_ASSERT_EQUAL_HEX16(0x4000, result);
}

static void test_read_buttons_all_4x4_positions(void)
{
    for (int i = 0; i < 16; i++) {
        mock_reset();
        mock_matrix_pressed_btn = i;
        uint16_t result = read_buttons();
        TEST_ASSERT_EQUAL_HEX16((uint16_t)(1U << i), result);
    }
}

static void test_read_buttons_simultaneous_multi_press(void)
{
    mock_reset();
    // Press two buttons simultaneously: Register Avanti (D9-D3: 0x0001) + Anilox Av Posteriore (D18-D4: 0x0020)
    mock_matrix_pressed_mask = BUTTON_REGISTER_AVANTI | BUTTON_ANILOX_AV_POSTERIORE;
    uint16_t result = read_buttons();
    TEST_ASSERT_EQUAL_HEX16(0x0021, result);

    mock_reset();
    // Press three buttons across different rows:
    // Register Indietro (0x0002) + Cliche Av Posteriore (0x0400) + Color Attacco (0x1000)
    mock_matrix_pressed_mask = BUTTON_REGISTER_INDIETRO | BUTTON_CLICHE_AV_POSTERIORE | BUTTON_COLOR_ATTACCO;
    result = read_buttons();
    TEST_ASSERT_EQUAL_HEX16(0x1402, result);

    mock_reset();
    // Press E-Stop together with Color Colore
    mock_matrix_pressed_mask = BUTTON_ESTOP_ACTIVE | BUTTON_COLOR_COLORE;
    result = read_buttons();
    TEST_ASSERT_EQUAL_HEX16(0xC000, result);
}

static void test_read_buttons_all_16_simultaneous(void)
{
    mock_reset();
    mock_matrix_pressed_mask = 0xFFFF;
    uint16_t result = read_buttons();
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, result);
}

static void test_buttons_enum_bitmask_consistency(void)
{
    // Register (Row 0: D9)
    TEST_ASSERT_EQUAL_HEX16(0x0001, BUTTON_REGISTER_AVANTI);     // D9 - D3
    TEST_ASSERT_EQUAL_HEX16(0x0002, BUTTON_REGISTER_INDIETRO);   // D9 - D4
    TEST_ASSERT_EQUAL_HEX16(0x0004, BUTTON_REGISTER_ABTERIORE);  // D9 - D5
    TEST_ASSERT_EQUAL_HEX16(0x0008, BUTTON_REGISTER_POSTERIORE); // D9 - D6

    // Anilox (Row 1: D18)
    TEST_ASSERT_EQUAL_HEX16(0x0010, BUTTON_ANILOX_AV_ANTERIORE);  // D18 - D3
    TEST_ASSERT_EQUAL_HEX16(0x0020, BUTTON_ANILOX_AV_POSTERIORE); // D18 - D4
    TEST_ASSERT_EQUAL_HEX16(0x0040, BUTTON_ANILOX_IN_ANTERIORE);  // D18 - D5
    TEST_ASSERT_EQUAL_HEX16(0x0080, BUTTON_ANILOX_IN_POSTERIORE); // D18 - D6

    // Cliche (Row 2: D19)
    TEST_ASSERT_EQUAL_HEX16(0x0100, BUTTON_CLICHE_AV_ANTERIORE);  // D19 - D3
    TEST_ASSERT_EQUAL_HEX16(0x0200, BUTTON_CLICHE_IN_ANTERIORE);  // D19 - D4
    TEST_ASSERT_EQUAL_HEX16(0x0400, BUTTON_CLICHE_AV_POSTERIORE); // D19 - D5
    TEST_ASSERT_EQUAL_HEX16(0x0800, BUTTON_CLICHE_IN_POSTERIORE); // D19 - D6

    // Color & E-Stop (Row 3: D20)
    TEST_ASSERT_EQUAL_HEX16(0x1000, BUTTON_COLOR_ATTACCO); // D20 - D3
    TEST_ASSERT_EQUAL_HEX16(0x2000, BUTTON_COLOR_STACCO);  // D20 - D4
    TEST_ASSERT_EQUAL_HEX16(0x4000, BUTTON_COLOR_COLORE);  // D20 - D5
    TEST_ASSERT_EQUAL_HEX16(0x8000, BUTTON_ESTOP_ACTIVE);  // D20 - D6
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
    TEST_ASSERT_EQUAL(RF24_PA_LEVEL_DEFAULT, mock_rf24_pa_level);
    TEST_ASSERT_EQUAL(RF24_DATARATE_DEFAULT, mock_rf24_data_rate);
    TEST_ASSERT_EQUAL_UINT8(5, mock_rf24_retries_delay);
    TEST_ASSERT_EQUAL(CHANNEL, mock_rf24_channel);
    TEST_ASSERT_TRUE(mock_rf24_ack_payload_enabled);
    TEST_ASSERT_TRUE(mock_rf24_dynamic_payloads_enabled);
    TEST_ASSERT_EQUAL_UINT8(15, mock_rf24_retries_count);
}

static void test_radio_send_safety_frame_success(void)
{
    mock_reset();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);

    uint8_t active_unit = 0;
    bool sent = radio_send_safety_frame_and_read_ack(&test_radio, 0x0001, 12000, TS_STAT_ON, 42, &active_unit);
    TEST_ASSERT_TRUE(sent);
    TEST_ASSERT_EQUAL(sizeof(ControlPacket), mock_rf24_tx_len);

    ControlPacket* tx = (ControlPacket*)mock_rf24_tx_buf;
    TEST_ASSERT_EQUAL_HEX16(0x0001, tx->buttons);
    TEST_ASSERT_TRUE(protocol_verify_buttons(tx->buttons, tx->buttons_inv));
    TEST_ASSERT_EQUAL_UINT16(12000, tx->battery_mv);
    TEST_ASSERT_EQUAL_UINT8(42, tx->seq);
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
}

static void test_radio_setup_detects_disconnected_chip(void)
{
    mock_reset();
    mock_rf24_chip_connected = false;
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);

    TEST_ASSERT_TRUE(error_is_active(ERROR_RADIO_NOT_CONNECTED));
    TEST_ASSERT_FALSE(radio_is_chip_connected(&test_radio));

    mock_rf24_chip_connected = true;
    TEST_ASSERT_TRUE(radio_is_chip_connected(&test_radio));
}

static void test_radio_software_telemetry_roundtrip(void)
{
    mock_reset();
    mock_rf24_auto_reply_telemetry = false;
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);

    TelemetryPacket staged_tel = {6, (uint8_t)TS_STAT_ON, 10, 0};
    memcpy(mock_rf24_ack_payload_buf, &staged_tel, sizeof(TelemetryPacket));
    mock_rf24_ack_payload_len = sizeof(TelemetryPacket);
    mock_rf24_ack_payload_available = true;

    uint8_t received_unit = 0;
    bool ok = radio_send_safety_frame_and_read_ack(
        &test_radio,
        0x0001,
        12000,
        TS_STAT_ON,
        10,
        &received_unit
    );

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_UINT8(6, received_unit);
    TEST_ASSERT_EQUAL(sizeof(ControlPacket), mock_rf24_tx_len);
}

static void test_radio_diagnostics_tracking(void)
{
    mock_reset();
    radio_reset_diagnostics();
    RF24 test_radio(CE_PIN, CSN_PIN);
    radio_setup(&test_radio);

    const RadioTxDiagnostics& d = radio_get_diagnostics();
    TEST_ASSERT_EQUAL_UINT32(0, d.tx_attempt_count);
    TEST_ASSERT_EQUAL_UINT32(0, d.tx_success_count);
    TEST_ASSERT_EQUAL_UINT32(0, d.tx_fail_count);

    // Successful transmission
    mock_rf24_write_result = true;
    uint8_t active_unit = 0;
    bool ok = radio_send_safety_frame_and_read_ack(&test_radio, 0, 12000, TS_STAT_ON, 1, &active_unit);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_UINT32(1, d.tx_attempt_count);
    TEST_ASSERT_EQUAL_UINT32(1, d.tx_success_count);
    TEST_ASSERT_EQUAL_UINT32(0, d.tx_fail_count);
    TEST_ASSERT_TRUE(d.last_tx_ok);
    TEST_ASSERT_EQUAL_UINT8(0, d.last_error_code);

    // Failed transmission
    mock_rf24_write_result = false;
    ok = radio_send_safety_frame_and_read_ack(&test_radio, 0, 12000, TS_STAT_ON, 2, &active_unit);
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_EQUAL_UINT32(2, d.tx_attempt_count);
    TEST_ASSERT_EQUAL_UINT32(1, d.tx_success_count);
    TEST_ASSERT_EQUAL_UINT32(1, d.tx_fail_count);
    TEST_ASSERT_FALSE(d.last_tx_ok);
    TEST_ASSERT_EQUAL_UINT8(2, d.last_error_code);
}

static void test_radio_lqi_scaled_convergence(void)
{
    mock_reset();
    radio_reset_diagnostics();
    TEST_ASSERT_EQUAL_UINT8(100, radio_get_lqi());

    radio_update_lqi(false);
    TEST_ASSERT_EQUAL_UINT8(95, radio_get_lqi());

    for (int i = 0; i < 10; i++)
    {
        radio_update_lqi(false);
    }
    TEST_ASSERT_TRUE(radio_get_lqi() < 60);

    for (int i = 0; i < 100; i++)
    {
        radio_update_lqi(true);
    }
    TEST_ASSERT_EQUAL_UINT8(100, radio_get_lqi());
}

static void test_display_show_packet_debug(void)
{
    mock_reset();
    RadioTxDiagnostics d;
    memset(&d, 0, sizeof(d));
    d.tx_attempt_count = 10;
    d.tx_success_count = 8;
    d.tx_fail_count = 2;
    d.last_tx_ok = true;
    d.last_ack_payload_ok = true;
    d.datarate_1mbps = 1;

    display_show_packet_debug(d, 78, 80, true);
    const uint8_t* ram = display_get_ram();

    TEST_ASSERT_BITS_HIGH(0x10, ram[8]); // Green indicator
    TEST_ASSERT_BITS_HIGH(0x10, ram[2]); // Yel1 indicator
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
// DISPLAY TESTS (ALC-40266RGY2 on TM1638)
// ============================================================
static void test_display_init(void)
{
    mock_reset();
    display_init();

    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram[6]); // Red fault indicator on GRID4
    TEST_ASSERT_BITS_HIGH(0xC0, ram[14]); // '----' top row on GRID8
}

static void test_display_digit_transposition_no_dp(void)
{
    mock_reset();
    display_clear();
    display_set_top("8   ");

    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x40, ram[8]);   // GRID5 (a)
    TEST_ASSERT_BITS_HIGH(0x40, ram[6]);   // GRID4 (b)
    TEST_ASSERT_BITS_HIGH(0x40, ram[2]);   // GRID2 (c)
    TEST_ASSERT_BITS_HIGH(0x40, ram[0]);   // GRID1 (d)
    TEST_ASSERT_BITS_HIGH(0x40, ram[4]);   // GRID3 (e)
    TEST_ASSERT_BITS_HIGH(0x40, ram[10]);  // GRID6 (f)
    TEST_ASSERT_BITS_HIGH(0x40, ram[14]);  // GRID8 (g)
    TEST_ASSERT_BITS_LOW(0x40, ram[12]);   // GRID7 (DP) must NOT be set!
}

static void test_display_no_comm_state(void)
{
    mock_reset();
    display_update(1, 60, TS_STAT_ON, false);

    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram[6]);
    TEST_ASSERT_BITS_LOW(0x10, ram[8]);
    TEST_ASSERT_BITS_HIGH(0xC0, ram[14]);
    TEST_ASSERT_BITS_HIGH(0x03, ram[15]);
    TEST_ASSERT_BITS_LOW(0xCF, ram[12]);
    TEST_ASSERT_BITS_LOW(0x03, ram[13]);
}

static void test_display_normal_run_state(void)
{
    mock_reset();
    display_update(3, 100, TS_STAT_ON, true);

    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram[8]);
    TEST_ASSERT_BITS_LOW(0x10, ram[6]);
    TEST_ASSERT_BITS_HIGH(0x20, ram[12]);
    TEST_ASSERT_BITS_HIGH(0x10, ram[14]);
}

static void test_display_18650_voltage_bottom_row(void)
{
    mock_reset();
    // 4150 mV should format as "4.15" on bottom row
    display_update(1, 100, TS_STAT_ON, true, 0, 4150);

    const uint8_t* ram = display_get_ram();
    // Digit 0 of bottom row is SEG4 (bit mask 0x08, low byte)
    // '4' has segments b (GRID4, ram[6]), c (GRID2, ram[2]), f (GRID6, ram[10]), g (GRID8, ram[14])
    TEST_ASSERT_BITS_HIGH(0x08, ram[6]);
    TEST_ASSERT_BITS_HIGH(0x08, ram[2]);
    TEST_ASSERT_BITS_HIGH(0x08, ram[10]);
    TEST_ASSERT_BITS_HIGH(0x08, ram[14]);
}

static void test_battery_bar_10_levels_and_indicator_isolation(void)
{
    mock_reset();
    display_clear();

    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_EQUAL_UINT8(0, ram[14] & 0x30);
    TEST_ASSERT_EQUAL_UINT8(0, ram[10] & 0x30);

    display_set_indicators(DISPLAY_IND_GREEN);
    TEST_ASSERT_BITS_HIGH(0x10, ram[8]);

    display_set_battery_bar(1);
    TEST_ASSERT_BITS_HIGH(0x10, ram[14]);
    TEST_ASSERT_BITS_LOW(0x10, ram[10]);
    TEST_ASSERT_BITS_LOW(0x20, ram[12]);
    TEST_ASSERT_BITS_HIGH(0x10, ram[8]);

    display_set_battery_bar(2);
    TEST_ASSERT_BITS_HIGH(0x10, ram[14]);
    TEST_ASSERT_BITS_HIGH(0x10, ram[10]);
    TEST_ASSERT_BITS_LOW(0x20, ram[12]);
    TEST_ASSERT_BITS_HIGH(0x10, ram[8]);

    display_set_indicators(DISPLAY_IND_RED);
    TEST_ASSERT_BITS_LOW(0x10, ram[8]);
    TEST_ASSERT_BITS_HIGH(0x10, ram[6]);
    TEST_ASSERT_BITS_HIGH(0x10, ram[14]);
    TEST_ASSERT_BITS_HIGH(0x10, ram[10]);

    display_set_battery_bar(10);
    TEST_ASSERT_BITS_HIGH(0x10, ram[6]);
    TEST_ASSERT_EQUAL_UINT8(0x30, ram[14] & 0x30);
    TEST_ASSERT_EQUAL_UINT8(0x30, ram[10] & 0x30);
    TEST_ASSERT_BITS_HIGH(0x20, ram[12]);
    TEST_ASSERT_BITS_HIGH(0x20, ram[8]);
}

static void test_display_radio_error_state(void)
{
    mock_reset();
    error_set(ERROR_RADIO_NOT_CONNECTED);
    display_update(1, 60, TS_STAT_ON, false);

    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram[6]);
    TEST_ASSERT_BITS_LOW(0x10, ram[8]);
    TEST_ASSERT_BITS_HIGH(0x40, ram[8]);
    TEST_ASSERT_BITS_HIGH(0x40, ram[0]);
    TEST_ASSERT_BITS_HIGH(0x40, ram[4]);
    TEST_ASSERT_BITS_HIGH(0x40, ram[10]);
    TEST_ASSERT_BITS_HIGH(0x40, ram[14]);
    TEST_ASSERT_BITS_HIGH(0x80, ram[4]);
    TEST_ASSERT_BITS_HIGH(0x80, ram[14]);
    TEST_ASSERT_BITS_HIGH(0x02, ram[5]);
    TEST_ASSERT_BITS_HIGH(0x02, ram[15]);
}

static void test_display_linking_countdown(void)
{
    mock_reset();

    display_show_linking(5);
    const uint8_t* ram5 = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x20, ram5[10]);
    TEST_ASSERT_BITS_LOW(0x20, ram5[8]);

    display_show_linking(1);
    const uint8_t* ram1 = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram1[14]);
    TEST_ASSERT_BITS_LOW(0x10, ram1[10]);
}

static void test_display_linking_debug(void)
{
    mock_reset();
    display_show_linking_debug(78, 2);
    const uint8_t* ram = display_get_ram();
    // Red indicator on GRID4 (ram[6] bit 4)
    TEST_ASSERT_BITS_HIGH(0x10, ram[6]);
    // Top row should have 'C' on S1_UP (GRID1, GRID3, GRID5: bits d, e, a)
    TEST_ASSERT_BITS_HIGH(0x40, ram[8]); // GRID5 (a)
    TEST_ASSERT_BITS_HIGH(0x40, ram[0]); // GRID1 (d)
    TEST_ASSERT_BITS_HIGH(0x40, ram[4]); // GRID3 (e)
}

// ============================================================
void setUp(void) {}
void tearDown(void) {}

int main(void)
{
    UNITY_BEGIN();

    // Protocol tests
    RUN_TEST(test_protocol_packet_sizes);
    RUN_TEST(test_protocol_verify_buttons_valid);
    RUN_TEST(test_protocol_verify_buttons_corrupted);
    RUN_TEST(test_protocol_control_packet_construction);

    // Header & Math tests
    RUN_TEST(test_mapfloat_maps_correctly);
    RUN_TEST(test_mapint_maps_correctly);
    RUN_TEST(test_mapint_battery_voltage_full);
    RUN_TEST(test_mapint_battery_voltage_empty);

    // Battery tests
    RUN_TEST(test_battery_init_sets_pullup);
    RUN_TEST(test_battery_charge_range);
    RUN_TEST(test_battery_voltage_full_formula);
    RUN_TEST(test_battery_voltage_empty_formula);
    RUN_TEST(test_battery_floating_pin_unconnected);

    // Buttons tests
    RUN_TEST(test_buttons_begin_initializes);
    RUN_TEST(test_read_buttons_returns_zero_when_idle);
    RUN_TEST(test_read_buttons_row0_col0);
    RUN_TEST(test_read_buttons_row4_col2);
    RUN_TEST(test_read_buttons_all_4x4_positions);
    RUN_TEST(test_read_buttons_simultaneous_multi_press);
    RUN_TEST(test_read_buttons_all_16_simultaneous);
    RUN_TEST(test_buttons_enum_bitmask_consistency);

    // Radio tests
    RUN_TEST(test_radio_setup_configures_safety);
    RUN_TEST(test_radio_setup_detects_disconnected_chip);
    RUN_TEST(test_radio_send_safety_frame_success);
    RUN_TEST(test_radio_channel_hopping);
    RUN_TEST(test_radio_software_telemetry_roundtrip);
    RUN_TEST(test_radio_diagnostics_tracking);
    RUN_TEST(test_radio_lqi_scaled_convergence);
    RUN_TEST(test_display_show_packet_debug);

    // Error tests
    RUN_TEST(test_error_handling);

    // Display tests
    RUN_TEST(test_display_init);
    RUN_TEST(test_display_digit_transposition_no_dp);
    RUN_TEST(test_display_no_comm_state);
    RUN_TEST(test_display_normal_run_state);
    RUN_TEST(test_display_18650_voltage_bottom_row);
    RUN_TEST(test_battery_bar_10_levels_and_indicator_isolation);
    RUN_TEST(test_display_radio_error_state);
    RUN_TEST(test_display_linking_countdown);
    RUN_TEST(test_display_linking_debug);

    return UNITY_END();
}
