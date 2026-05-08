#ifndef _H_GLCD
#define _H_GLCD

#include <U8glib.h>
#include "header.h"

// Change these pins to match your GLCD wiring.

void glcd_init();
void glcd_update(uint8_t active_unit, uint8_t battery_level, ts_status status, bool comm_ok);

#endif
