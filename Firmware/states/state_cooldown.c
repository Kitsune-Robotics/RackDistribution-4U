#include "state_internal.h"

#include "analog.h"
#include "indicators.h"
#include "parameters.h"
#include "tusb.h"
#include "usb_vbus.h"

static TickType_t cold_since;

void state_cooldown_tick(TickType_t now) {
  if (!usb_vbus_present() || tud_ready()) {
    state_goto(STATE_RUN);
    return;
  }

  const float t = analog_control_c();
  if (t < analog_ambient_c() + COOLDOWN_COLD_ABOVE_AIR_C) {
    if (cold_since == 0) {
      cold_since = now;
    } else if ((now - cold_since) >= pdMS_TO_TICKS(STATE_COOLDOWN_COLD_MS)) {
      state_goto(STATE_STANDBY);
    }
  } else {
    cold_since = 0;
  }
}

void state_cooldown_entry(void) {
  cold_since = 0;
  indicator_off(&g_indicators.standby);
  indicator_flash(&g_indicators.cooldown, COLOR_YELLOW);
  indicator_off(&g_indicators.control);
}
