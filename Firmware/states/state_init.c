#include "state_internal.h"

#include "indicators.h"
#include "parameters.h"
#include "tusb.h"

void state_init_tick(TickType_t now) {
  // Wait till init time is over
  if ((now - state_entered()) >= pdMS_TO_TICKS(STATE_INIT_MS)) {
    // Go to standby if usb is not connected, otherwise go to run
    state_goto(tud_ready() ? STATE_RUN : STATE_STANDBY);
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
