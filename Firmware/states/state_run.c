#include "state_internal.h"

#include "hid.h"
#include "indicators.h"
#include "parameters.h"
#include "tusb.h"
#include "usb_vbus.h"

static TickType_t usb_lost_at;

static void update_control_light(void) {
  if (!usb_vbus_present()) {
    indicator_fast_flash(&g_indicators.control, COLOR_RED);
  } else if (hid_consumed()) {
    indicator_solid(&g_indicators.control, COLOR_GREEN);
  } else if (tud_ready()) {
    indicator_solid(&g_indicators.control, COLOR_YELLOW);
  } else {
    indicator_flash(&g_indicators.control, COLOR_YELLOW);
  }
}

void state_run_tick(TickType_t now) {
  update_control_light();

  if (!usb_vbus_present()) {
    usb_lost_at = 0;
    return;
  }

  if (tud_ready()) {
    usb_lost_at = 0;
  } else if (usb_lost_at == 0) {
    usb_lost_at = now;
  } else if ((now - usb_lost_at) >= pdMS_TO_TICKS(STATE_USB_LOST_MS)) {
    state_goto(STATE_COOLDOWN);
  }
}

void state_run_entry(void) {
  usb_lost_at = 0;
  indicator_off(&g_indicators.standby);
  indicator_off(&g_indicators.cooldown);
  update_control_light();
}
