#include "state_internal.h"

#include "analog.h"
#include "indicators.h"
#include "parameters.h"
#include "tusb.h"

static TickType_t cold_since;

void state_cooldown_tick(TickType_t now) {
  if (tud_ready()) {
    // Go back to RUN if usb comes back
    state_goto(STATE_RUN);
    return;
  }

  // Check if the temperature is below the cold threshold
  if (analog_tsensor1_c() < COOLDOWN_COLD_C) {
    if (cold_since == 0) {
      cold_since = now;
    } else if ((now - cold_since) >= pdMS_TO_TICKS(STATE_COOLDOWN_COLD_MS)) {
      state_goto(STATE_STANDBY);
    }
  } else {
    cold_since = 0;
  }

  // Check for warnings
  if (analog_tsensor1_c() > A_LITTLE_HOT_C) {
    indicator_flash(&g_indicators.a_little_hot, COLOR_RED);
  } else {
    indicator_off(&g_indicators.a_little_hot);
  }
}

void state_cooldown_entry(void) {
  cold_since = 0;
  indicator_off(&g_indicators.standby);
  indicator_flash(&g_indicators.cooldown, COLOR_YELLOW);
  indicator_off(&g_indicators.control);
}
