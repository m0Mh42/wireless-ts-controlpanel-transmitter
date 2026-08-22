#include <Arduino.h>
#include "radio.h"
#include "header.h"
#include "buttons.h"
#include "battery.h"
#include "display.h"
#include "error.h"
#include "safety_protocol.h"

// Communication timing constants
#define COMM_TIMEOUT_MS 250UL
#define MAX_CONSECUTIVE_TX_FAILS 5

// Local Global-Variables
RF24 radio(CE_PIN, CSN_PIN);
ts_status local_ts_status = TS_STAT_ON;
uint16_t local_buttons = 0;
uint8_t battery_cap = 0;
uint16_t battery_mv = 0;
uint8_t local_active_unit = 0;
bool local_comm_ok = false;
unsigned long last_comm_ms = 0;
uint32_t monotonic_tx_seq = 1;
uint32_t last_rx_seq = 0;
uint8_t consecutive_tx_failures = 0;
bool battery_low_logged = false;

void setup()
{
#if DEBUG
    Serial.begin(9600);
    while (!Serial)
        ;
    Serial.println(F("[SYSTEM] UTECO Transmitter Safe Pendant Initializing..."));
#endif

    // Setup RF24 Radio
    radio_setup(&radio);

    // Initialize the Matrix Button scanner
    buttons_begin();

    // Initialize the TM1638 Display
    display_init();
    display_update(local_active_unit, battery_cap, local_ts_status, local_comm_ok);

#if DEBUG
    Serial.println(F("[SYSTEM] Transmitter Pendant Ready."));
#endif
}

void loop()
{
    unsigned long now = millis();

    // 1. Read 15-Button Matrix (16-bit bitmask)
    local_buttons = read_buttons();

    // 2. Read Battery Telemetry
    battery_cap = battery_charge();
    battery_mv = battery_millivolts();

    if (battery_cap < 50)
    {
        if (!battery_low_logged)
        {
#if DEBUG
            Serial.println(F("[BATTERY] Battery Low Warning"));
#endif
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

    // 3. Transmit Deterministic Black Channel Safety Frame (Cyclic 20 ms)
    bool sent = radio_send_safety_frame(
        &radio,
        local_buttons,
        battery_mv,
        local_ts_status,
        monotonic_tx_seq++
    );

    if (sent)
    {
        error_clear(ERROR_TRANSMIT_FAILED);
        last_comm_ms = now;
        consecutive_tx_failures = 0;
    }
    else
    {
        error_set(ERROR_TRANSMIT_FAILED);
        consecutive_tx_failures++;

        // Adaptive frequency hopping after 5 consecutive dropped frames (100 ms)
        if (consecutive_tx_failures >= MAX_CONSECUTIVE_TX_FAILS)
        {
#if DEBUG
            Serial.println(F("[RADIO] Consecutive TX fails -> Hopping to next Wi-Fi Guard Channel"));
#endif
            radio_hop_next_channel(&radio);
            consecutive_tx_failures = 0;
        }
    }

    // 4. Read Telemetry / ACK Feedback from Receiver
    if (radio_read_telemetry(&radio, &local_active_unit, &last_rx_seq))
    {
        last_comm_ms = now;
    }

    // 5. Link Health & Status Monitoring
    local_comm_ok = (now - last_comm_ms) < COMM_TIMEOUT_MS;
    if (!local_comm_ok)
    {
        error_set(ERROR_NO_COMMUNICATION);
    }
    else
    {
        error_clear(ERROR_NO_COMMUNICATION);
    }

    // 6. Update TM1638 Display
    display_update(local_active_unit, battery_cap, local_ts_status, local_comm_ok);

    // 7. Error LED and diagnostics handling
    transaction_error_handling();

    delay(20);
}
