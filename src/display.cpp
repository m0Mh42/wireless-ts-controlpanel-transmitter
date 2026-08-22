#include "display.h"
#include "buttons.h" // For TM1638 pin definitions
#include <TM1638.h>

// Initialize TM1638 for 10 digits as requested.
// Since the imported library already has common anode support, we initialize it directly.
static TM1638 display_module(DISPLAY_TM1638_DIO_PIN, DISPLAY_TM1638_CLK_PIN, DISPLAY_TM1638_STB_PIN, 10, false, 0);

void display_init()
{
    // Initialize the display
    display_module.begin(false, 0);
    display_module.clearDisplay();
}

void display_update(uint8_t active_unit, uint8_t battery_level, ts_status status, bool comm_ok)
{
    // Placeholder logic for the custom 7-segment display behavior.
    // To be clarified by user.
    // For now, let's just display the active unit on the first digit if comm is ok.
    if (comm_ok && status == TS_STAT_ON) {
        display_module.setDisplayDigit(active_unit, 0, false);
    } else if (status == TS_STAT_BATTERYLOW) {
        display_module.setDisplayToString("BAT LOW   ");
    } else {
        display_module.clearDisplay();
    }
}
