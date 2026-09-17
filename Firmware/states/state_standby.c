#include "state_internal.h"

#include "indicators.h"
#include "tusb.h"
#include "usb_vbus.h"

void state_standby_tick(TickType_t now) {
  (void)now;
  // If usb NOT present, or we're ready to go. Go to run
  if (!usb_vbus_present() || tud_ready()) {
    state_goto(STATE_RUN);
  }
}

void state_standby_entry(void) {
  indicator_solid(&g_indicators.standby, COLOR_YELLOW);
  indicator_off(&g_indicators.cooldown);
  indicator_off(&g_indicators.control);
}
