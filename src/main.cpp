#include <Arduino.h>
#include "radio.h"
#include "header.h"
#include "buttons.h"
#include "battery.h"

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
uint64_t local_seq = 0;
ts_status local_ts_status = TS_STAT_OFF;
uint8_t local_buttons = 0, battery_cap = 0;
bool flag = false;

void setup()
{
  if (DEBUG || TEST)
  {
    Serial.begin(9600);
    while (!Serial)
      ; // Wait for USB Serial
  }

  // TODO Check Battery Voltage on Startup
  battery_cap = battery_charge();
  if (battery_cap < 10)
  {
    Serial.println("Battery is low");
    while (true)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
    }
  }

  // Setup Radio
  radio_setup(&radio);
}

void loop()
{
  while (TEST)
  {
    transaction_unit test_transaction_unit;
    bool ack = false;
    test_transaction_unit.command = COMM_START_TX;
    test_transaction_unit.seq = 0;
    test_transaction_unit.buttons = 0U;
    radio.write(&test_transaction_unit, sizeof(test_transaction_unit));
    while (true)
    {
      if (radio.available())
      {
        radio.read(&test_transaction_unit, sizeof(transaction_unit));
        if (test_transaction_unit.command == COMM_ACK && test_transaction_unit.seq == 1)
        {
          ack = true;
          local_seq = 2;
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
        test_transaction_unit.seq = local_seq;
        test_transaction_unit.buttons = 0;
        radio.write(&test_transaction_unit, sizeof(test_transaction_unit));
        ack = false;
        local_seq++;
      }
      if (radio.available())
      {
        radio.read(&test_transaction_unit, sizeof(transaction_unit));
        if (test_transaction_unit.command == COMM_ACK && test_transaction_unit.seq == local_seq)
        {
          ack = true;
          local_seq++;
        }
      }
      if (digitalRead(13))
      {
        test_transaction_unit.command = COMM_STOP_TX;
        test_transaction_unit.seq = local_seq;
        test_transaction_unit.buttons = 0;
        radio.write(&test_transaction_unit, sizeof(test_transaction_unit));
        break;
      }
      delay(1000);
    }
  }

  // Start Communication
  if (local_ts_status == TS_STAT_OFF)
  {
    radio_start_ts(&radio, &local_transaction_unit, &local_seq);
    local_ts_status = TS_STAT_ON;
  }

  // Read Buttons
  local_buttons = read_buttons();

  // Communicate if Buttons were Pressed
  if (local_buttons > 0)
  {
    // Communicate local_transaction_unit
    local_transaction_unit.buttons = local_buttons;
    local_transaction_unit.command = COMM_BUTTON;
    radio_transact(&radio, &local_transaction_unit, &local_seq);
  }

  // TODO Check Battery Voltage
  battery_cap = battery_charge();
  if (battery_cap < 50 && flag == false)
  {
    Serial.println("Battery is low");
    flag = true;
  }
}
