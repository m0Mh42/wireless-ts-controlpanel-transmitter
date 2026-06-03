#include "error.h"

error_type_enum error_state = ERROR_NONE;
uint8_t error_counter = 0;

static unsigned long last_led_toggle = 0;
static bool led_state = false;

void error_set(error_type_enum error)
{
    error_state = error;
    error_counter++;
}

void error_clear(error_type_enum error)
{
    if (error_state == error)
    {
        error_state = ERROR_NONE;
    }
}

bool error_is_active(error_type_enum error)
{
    return error_state == error;
}

void error_update_led()
{
    if (error_state == ERROR_NONE)
    {
        digitalWrite(LED_BUILTIN, LOW);
        led_state = false;
        return;
    }

    unsigned long now = millis();
    unsigned long interval = 1000;

    switch (error_state)
    {
    case ERROR_RADIO_NOT_CONNECTED:
        interval = 100;
        break;
    case ERROR_TRANSMIT_FAILED:
        interval = 250;
        break;
    case ERROR_BATTERY_LOW:
        interval = 500;
        break;
    case ERROR_NO_COMMUNICATION:
        interval = 750;
        break;
    default:
        interval = 1000;
        break;
    }

    if (now - last_led_toggle >= interval)
    {
        last_led_toggle = now;
        led_state = !led_state;
        digitalWrite(LED_BUILTIN, led_state ? HIGH : LOW);
    }
}

void transaction_error_handling()
{
    if (error_state != ERROR_NONE && error_counter > 0)
    {
        if (DEBUG)
        {
            Serial.print("ERR: state=");
            Serial.print((int)error_state);
            Serial.print(" count=");
            Serial.println(error_counter);
        }
    }
    error_update_led();
}