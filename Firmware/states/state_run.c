#include "state_internal.h"

#include "analog.h"
#include "indicators.h"
#include "parameters.h"
#include "tusb.h"

// When USB disconnected
static TickType_t usb_lost_at;

// More robust way to see if the USB is actually draining TX
static bool cdc_open(void) {
  return tud_cdc_connected() && tud_cdc_write_available() > 0;
}

static void update_control_light(void) {
  if (cdc_open()) {
    indicator_solid(&g_indicators.control, COLOR_GREEN);
  } else if (tud_ready()) {
    indicator_solid(&g_indicators.control, COLOR_YELLOW);
  } else {
    indicator_flash(&g_indicators.control, COLOR_YELLOW);
  }
}

void state_run_tick(TickType_t now) {
  update_control_light();

  if (tud_ready()) {
    usb_lost_at = 0;
  } else if (usb_lost_at == 0) {
    usb_lost_at = now;
  } else if ((now - usb_lost_at) >= pdMS_TO_TICKS(STATE_USB_LOST_MS)) {
    state_goto(STATE_COOLDOWN);
    return;
  }

  // Check for warnings
  if (analog_tsensor1_c() > A_LITTLE_HOT_C) {
    indicator_flash(&g_indicators.a_little_hot, COLOR_RED);
  } else {
    indicator_off(&g_indicators.a_little_hot);
  }
}

void state_run_entry(void) {
  usb_lost_at = 0;
  indicator_off(&g_indicators.standby);
  indicator_off(&g_indicators.cooldown);
  update_control_light();
}
