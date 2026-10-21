#include <Arduino.h>
#ifdef __AVR__
#include <avr/wdt.h>
#endif
#include "radio.h"
#include "header.h"
#include "buttons.h"
#include "battery.h"
#include "display.h"
#include "error.h"
#include "protocol.h"

// Communication timing constants
#define COMM_TIMEOUT_MS 200UL

// Local Global-Variables
ts_status local_ts_status = TS_STAT_ON;
uint16_t local_buttons = 0;
uint8_t battery_cap = 0;
uint16_t battery_mv = 0;
uint8_t local_active_unit = 0;
bool local_comm_ok = false;
unsigned long last_comm_ms = 0;
uint32_t monotonic_tx_seq = 1;
uint8_t consecutive_tx_failures = 0;
bool battery_low_logged = false;

void setup()
{
#ifdef __AVR__
    wdt_disable();
#endif

    // 0. Immediately park nRF24 SPI bus lines to prevent floating-pin bus corruption during display init
    pinMode(CSN_PIN, OUTPUT);
    digitalWrite(CSN_PIN, HIGH);
    pinMode(CE_PIN, OUTPUT);
    digitalWrite(CE_PIN, LOW);

    // 1. Initialize the TM1638 Display FIRST so the operator gets immediate visual output
    display_init();

    // 2. Initialize Battery ADC pin with internal pullup
    battery_init();

    // 3. Initialize the Matrix Button scanner
    buttons_begin();

    // 4. Setup RF24 Radio
    radio_setup();
}

void loop()
{
    unsigned long now = millis();

    // 1. Hardware Connection Health Check of nRF24 Transceiver (with debouncing)
    static uint8_t radio_disconnect_count = 0;
    static bool radio_hardware_error = false;

    if (!radio_is_chip_connected())
    {
        if (radio_disconnect_count < 20)
        {
            radio_disconnect_count++;
        }
        else
        {
            radio_hardware_error = true;
            error_set(ERROR_RADIO_NOT_CONNECTED);
        }
    }
    else
    {
        radio_disconnect_count = 0;
        radio_hardware_error = false;
        error_clear(ERROR_RADIO_NOT_CONNECTED);
    }

    if (radio_hardware_error)
    {
        local_comm_ok = false;
        static unsigned long last_err_display_ms = 0;
        if (now - last_err_display_ms >= 50UL)
        {
            last_err_display_ms = now;
            display_show_radio_error();
        }

        // Auto-recovery: Re-initialize nRF24 every 500ms when module is reconnected
        static unsigned long last_reinit_ms = 0;
        if (now - last_reinit_ms >= 500UL)
        {
            last_reinit_ms = now;
            radio_setup();
            if (radio_is_chip_connected())
            {
                radio_hardware_error = false;
                radio_disconnect_count = 0;
                error_clear(ERROR_RADIO_NOT_CONNECTED);
            }
        }

        delay(20);
        return;
    }

    // 2. Read 15-Button Matrix (16-bit bitmask)
    local_buttons = read_buttons();

    // 3. Read Battery Telemetry
    battery_mv = battery_millivolts();
    bool bat_connected = battery_is_connected();

    if (!bat_connected)
    {
        // Battery pin is floating or unconnected (bench power mode)
        battery_cap = 100;
        local_ts_status = TS_STAT_ON;
        battery_low_logged = false;
        error_clear(ERROR_BATTERY_LOW);
    }
    else
    {
        // Map 0..255 battery charge to 0..100 percentage
        battery_cap = (uint8_t)(((uint16_t)battery_charge() * 100) / 255);
        if (battery_cap < 20)
        {
            if (!battery_low_logged)
            {
                battery_low_logged = true;
            }
            local_ts_status = TS_STAT_BATTERYLOW;
            error_set(ERROR_BATTERY_LOW);
        }
        else
        {
            battery_low_logged = false;
            local_ts_status = TS_STAT_ON;
            error_clear(ERROR_BATTERY_LOW);
        }
    }

    // 4. Transmit Deterministic Control Packet (Cyclic 20 ms)
    ControlPacket pkt;
    pkt.buttons = local_buttons;
    pkt.buttons_inv = (uint16_t)(~local_buttons);
    pkt.battery_mv = battery_mv;
    pkt.status = (uint8_t)local_ts_status;
    pkt.seq = (uint8_t)(monotonic_tx_seq++ & 0xFF);

    TelemetryPacket tel = {0, 0, 0, 0};
    bool tel_received = false;
    bool sent = radio_send_control(pkt, &tel, &tel_received);

    if (sent)
    {
        error_clear(ERROR_TRANSMIT_FAILED);
        last_comm_ms = now;
        consecutive_tx_failures = 0;
        if (tel_received && tel.active_unit > 0)
        {
            local_active_unit = tel.active_unit;
        }
    }
    else
    {
        error_set(ERROR_TRANSMIT_FAILED);
        consecutive_tx_failures++;
    }

    // 5. Link Health & Status Monitoring
    local_comm_ok = (now - last_comm_ms) < COMM_TIMEOUT_MS;
    if (local_comm_ok)
    {
        error_clear(ERROR_NO_COMMUNICATION);
    }
    else
    {
        error_set(ERROR_NO_COMMUNICATION);
    }

    // 6. Update TM1638 Display (rate-limited to 40 ms / 25 Hz for flicker-free display)
    static unsigned long last_display_ms = 0;
    if (now - last_display_ms >= 40UL)
    {
        last_display_ms = now;
#if (defined(TEST) && TEST) || (defined(DEBUG) && DEBUG)
        display_show_packet_debug(radio_get_diagnostics(), radio_get_channel(), radio_get_lqi(), !radio_hardware_error);
#else
        if (local_comm_ok)
        {
            display_update(local_active_unit, battery_cap, local_ts_status, true, local_buttons, battery_mv, error_state);
        }
        else
        {
            display_show_linking(0);
        }
#endif
    }

    // 7. Error LED and diagnostics handling (Orange LED on pin 13 lights up on any button press)
    transaction_error_handling();
    if (local_buttons != 0)
    {
        digitalWrite(LED_BUILTIN, HIGH);
    }

    unsigned long cycle_elapsed = millis() - now;
    if (cycle_elapsed < TX_CYCLE_PERIOD_MS)
    {
        delay(TX_CYCLE_PERIOD_MS - cycle_elapsed);
    }
}
