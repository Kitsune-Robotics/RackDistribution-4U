#include "state_internal.h"

#include "indicators.h"
#include "tusb.h"

void state_standby_tick(TickType_t now) {
  (void)now;
  if (tud_ready()) {
    // If the usb is connected, go to run
    state_goto(STATE_RUN);
  }
}

void state_standby_entry(void) {
  indicator_solid(&g_indicators.standby, COLOR_YELLOW);
  indicator_off(&g_indicators.cooldown);
  indicator_off(&g_indicators.control);
}
