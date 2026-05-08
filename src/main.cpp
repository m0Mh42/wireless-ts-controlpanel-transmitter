#include <Arduino.h>
#include "radio.h"
#include "header.h"
#include "buttons.h"
#include "battery.h"
#include "glcd.h"

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

// Local Global-Variables
RF24 radio(CE_PIN, CSN_PIN); // RF24 Radio
transaction_unit local_transaction_unit;
ts_status local_ts_status = TS_STAT_OFF;
uint8_t local_buttons = 0, battery_cap = 0, local_active_unit = 0;
bool local_comm_ok = false;
bool flag = false;

void setup()
{
  if (DEBUG || TEST)
  {
    Serial.begin(9600);
    while (!Serial)
      ; // Wait for USB Serial
  }

  // TODO Display Battery Low Error on Startup
  battery_cap = battery_charge();
  if (battery_cap < 10)
  {
    Serial.println("Battery is low");
    uint32_t time_now = millis();
    while (millis() - time_now < 5)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
    }
  }

  // Setup Radio
  radio_setup(&radio);

  // Initialize the GLCD and show the first status screen.
  glcd_init();
  glcd_update(local_active_unit, battery_cap, local_ts_status, local_comm_ok);
}

void loop()
{
  while (TEST)
  {
    transaction_unit test_transaction_unit;
    bool ack = false;
    test_transaction_unit.buttons = 0U;
    test_transaction_unit.active_unit = 0U;
    radio.write(&test_transaction_unit, sizeof(test_transaction_unit));
    while (true)
    {
      if (radio.available())
      {
        radio.read(&test_transaction_unit, sizeof(transaction_unit));
        if (test_transaction_unit.command == COMM_DATA)
        {
          ack = true;
          break;
        }
      }
      delay(250);
    }
    while (true)
    {
      if (ack)
      {
        test_transaction_unit.command = COMM_BUTTON;
        test_transaction_unit.buttons = 0;
        radio.write(&test_transaction_unit, sizeof(test_transaction_unit));
        ack = false;
      }
      if (radio.available())
      {
        radio.read(&test_transaction_unit, sizeof(transaction_unit));
        if (test_transaction_unit.command == COMM_DATA)
        {
          ack = true;
        }
      }
      delay(1000);
    }
  }

  // Read Buttons
  local_buttons = read_buttons();

  // Communicate if Buttons were Pressed
  bool current_comm = false;
  if (local_buttons > 0)
  {
    // Communicate local_transaction_unit
    local_transaction_unit.buttons = local_buttons;
    local_transaction_unit.command = COMM_BUTTON;
    local_transaction_unit.active_unit = local_active_unit;
    current_comm = radio_transact(&radio, &local_transaction_unit);
  }

  // Read Radio Data
  bool radio_received = radio_read(&radio, &local_transaction_unit);
  if (radio_received)
  {
    current_comm = true;
    if (local_transaction_unit.command == COMM_DATA)
    {
      local_active_unit = local_transaction_unit.active_unit;
    }
  }

  local_comm_ok = current_comm;

  // Update battery and status
  battery_cap = battery_charge();
  if (battery_cap < 50)
  {
    if (!flag)
    {
      Serial.println("Battery is low");
      flag = true;
    }
    local_ts_status = TS_STAT_BATTERYLOW;
  }
  else
  {
    local_ts_status = TS_STAT_ON;
  }

  glcd_update(local_active_unit, battery_cap, local_ts_status, local_comm_ok);

  delay(100); // Small delay to prevent busy loop
}
