#include "state_internal.h"

#include "indicators.h"
#include "parameters.h"
#include "tusb.h"
#include "usb_vbus.h"

void state_init_tick(TickType_t now) {
  if ((now - state_entered()) < pdMS_TO_TICKS(STATE_INIT_MS)) {
    return;
  }
  if (!usb_vbus_present() || tud_ready()) {
    state_goto(STATE_RUN);
  } else {
    state_goto(STATE_STANDBY);
  }
}

void state_init_entry(void) {
  indicator_flash(&g_indicators.standby, COLOR_WHITE);
  indicator_solid(&g_indicators.cooldown, COLOR_YELLOW);
  indicator_solid(&g_indicators.a_little_hot, COLOR_RED);
  indicator_solid(&g_indicators.control, COLOR_GREEN);
  indicator_solid(&g_indicators.low_coolant, COLOR_RED);
  indicator_solid(&g_indicators.low_pump_speed, COLOR_RED);
  indicator_solid(&g_indicators.low_flow, COLOR_RED);
  indicator_solid(&g_indicators.low_fan_speed, COLOR_RED);
}
