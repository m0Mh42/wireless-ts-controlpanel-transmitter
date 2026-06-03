#ifndef _H_GLCD
#define _H_GLCD

#include <U8glib.h>
#include "header.h"

#define GLCD_COMM_D0 0
#define GLCD_COMM_D1 1
#define GLCD_COMM_D2 2
#define GLCD_COMM_D3 3
#define GLCD_COMM_D4 4
#define GLCD_COMM_D5 5
#define GLCD_COMM_D6 6
#define GLCD_COMM_D7 7
#define GLCD_COMM_EN A0
#define GLCD_COMM_CS1 A1
#define GLCD_COMM_CS2 A2
#define GLCD_COMM_DI A3

void glcd_init();
void glcd_update(uint8_t active_unit, uint8_t battery_level, ts_status status, bool comm_ok);

#endif
