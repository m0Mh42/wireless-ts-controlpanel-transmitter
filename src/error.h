#ifndef _H_ERROR
#define _H_ERROR

#include "header.h"

// Bit-flag error codes — multiple errors can be active simultaneously.
typedef enum error_type_enum
{
    ERROR_NONE              = 0,
    ERROR_RADIO_NOT_CONNECTED = 1,
    ERROR_TRANSMIT_FAILED   = 2,
    ERROR_BATTERY_LOW       = 4,
    ERROR_NO_COMMUNICATION  = 8
} error_type_enum;

// Bitmask of currently active errors (OR of error_type_enum values).
extern uint8_t error_state;
// Total error-set calls since boot; saturates at 65535.
extern uint16_t error_counter;

void error_set(error_type_enum error);
void error_clear(error_type_enum error);
bool error_is_active(error_type_enum error);
void transaction_error_handling();
void error_update_led();

#endif
