#ifndef _H_ERROR
#define _H_ERROR

#include "header.h"

typedef enum error_type_enum
{
    ERROR_NONE = 0,
    ERROR_RADIO_NOT_CONNECTED,
    ERROR_TRANSMIT_FAILED,
    ERROR_BATTERY_LOW,
    ERROR_NO_COMMUNICATION
} error_type_enum;

extern error_type_enum error_state;
extern uint8_t error_counter;

void error_set(error_type_enum error);
void error_clear(error_type_enum error);
bool error_is_active(error_type_enum error);
void transaction_error_handling();
void error_update_led();

#endif
