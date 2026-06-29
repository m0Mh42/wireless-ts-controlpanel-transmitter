#include <Arduino.h>
#include "radio.h"
#include "header.h"
#include "buttons.h"
#include "battery.h"
#include "glcd.h"
#include "error.h"

/*

  Program for TS Transmitter

  Module // Arduino UNO //         ESP32         //       ESP8266
    GND    ->   GND            ->  GND           ->       GND
    Vcc    ->   3.3V(External) ->  3.3V          ->       3.3V
    CE     ->   D9             ->  D4            ->       D4
    CSN    ->   D10            ->  D5            ->       D8
    CLK    ->   D13            ->  D18           ->       D5
    MOSI   ->   D11            ->  D23           ->       D7
    MISO   ->   D12            ->  D19           ->       D6

*/

// No-activity timeout before the link is considered lost (ms).
#define COMM_TIMEOUT_MS 500UL

// Local Global-Variables
RF24 radio(CE_PIN, CSN_PIN); // RF24 Radio
transaction_unit local_transaction_unit;
ts_status local_ts_status = TS_STAT_OFF;
uint8_t local_buttons = 0;
uint8_t battery_cap = 0, local_active_unit = 0;
bool local_comm_ok = false;
unsigned long last_comm_ms = 0;  // millis() of the last successful tx or rx
bool battery_low_logged = false;

void setup()
{
#if DEBUG || TEST
    Serial.begin(9600);
    while (!Serial)
        ; // Wait for USB Serial
#endif

    // Setup Radio
    radio_setup(&radio);

    // Initialize the TM1638 button scanner
    buttons_begin();

    // Initialize the GLCD and show the first status screen.
    glcd_init();
    glcd_update(local_active_unit, battery_cap, local_ts_status, local_comm_ok);
}

#if TEST

static void test_mode()
{
    transaction_unit test_transaction_unit;
    bool ack = false;
    test_transaction_unit.buttons = 0U;
    test_transaction_unit.active_unit = 0U;
    test_transaction_unit.command = COMM_START_TX;
    radio_transact(&radio, &test_transaction_unit);
    while (true)
    {
        if (radio_read(&radio, &test_transaction_unit))
        {
            if (test_transaction_unit.command == COMM_ACK)
            {
                ack = true;
                break;
            }
        }
        delay(100);
    }
    local_buttons = 0;
    while (true)
    {
        if (ack)
        {
            if (digitalRead(2))
            {
                test_transaction_unit.command = COMM_STOP_TX;
                test_transaction_unit.buttons = 0;
                radio_transact(&radio, &test_transaction_unit);
                while (true)
                {
                    error_set(ERROR_NONE);
                    delay(100);
                }
            }

            test_transaction_unit.command = COMM_BUTTON;
            test_transaction_unit.buttons = local_buttons++;
            radio_transact(&radio, &test_transaction_unit);
            ack = false;
        }
        if (radio_read(&radio, &test_transaction_unit))
        {
            if (test_transaction_unit.command == COMM_ACK)
            {
                ack = true;
            }
        }
        delay(250);
    }
}

#endif

void loop()
{
#if TEST
    test_mode();
    return;
#endif

    // Read Buttons
    local_buttons = read_buttons();

    // Communicate if Buttons were Pressed
    if (local_buttons > 0)
    {
        local_transaction_unit.buttons = local_buttons;
        local_transaction_unit.command = COMM_BUTTON;
        local_transaction_unit.active_unit = local_active_unit;
        bool sent = radio_transact(&radio, &local_transaction_unit);
        if (!sent)
        {
            error_set(ERROR_TRANSMIT_FAILED);
        }
        else
        {
            error_clear(ERROR_TRANSMIT_FAILED);
            last_comm_ms = millis();
        }
    }

    // Read Radio Data
    if (radio_read(&radio, &local_transaction_unit))
    {
        last_comm_ms = millis();
        if (local_transaction_unit.command == COMM_DATA)
        {
            local_active_unit = local_transaction_unit.active_unit;
        }
    }

    // Link is healthy if we had successful activity within the timeout window.
    local_comm_ok = (millis() - last_comm_ms) < COMM_TIMEOUT_MS;
    if (!local_comm_ok)
    {
        error_set(ERROR_NO_COMMUNICATION);
    }
    else
    {
        error_clear(ERROR_NO_COMMUNICATION);
    }

    // Update battery and status
    battery_cap = battery_charge();
    if (battery_cap < 50)
    {
        if (!battery_low_logged)
        {
            if (DEBUG)
            {
                Serial.println("Battery is low");
            }
            battery_low_logged = true;
        }
        local_ts_status = TS_STAT_BATTERYLOW;
    }
    else
    {
        battery_low_logged = false;
        local_ts_status = TS_STAT_ON;
    }

    glcd_update(local_active_unit, battery_cap, local_ts_status, local_comm_ok);
    transaction_error_handling();

    delay(100);
}
