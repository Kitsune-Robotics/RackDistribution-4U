#include "state_internal.h"

#include "analog.h"
#include "fans.h"
#include "hardware/gpio.h"
#include "indicators.h"
#include "parameters.h"
#include "pindefs.h"
#include "speaker.h"
#include "usb_vbus.h"

#include <stdbool.h>

static volatile system_state_t g_state = STATE_INIT;
static TickType_t g_entered;

typedef struct {
  void (*tick)(TickType_t now);
  void (*entry)(void);
} state_ops_t;

static const state_ops_t k_ops[] = {
    [STATE_INIT] = {state_init_tick, state_init_entry},
    [STATE_STANDBY] = {state_standby_tick, state_standby_entry},
    [STATE_COOLDOWN] = {state_cooldown_tick, state_cooldown_entry},
    [STATE_RUN] = {state_run_tick, state_run_entry},
};

system_state_t state_get(void) { return g_state; }

TickType_t state_entered(void) { return g_entered; }

void state_goto(system_state_t next) {
  g_state = next;
  g_entered = xTaskGetTickCount();
  gpio_put(LOAD_EN_PIN, next != STATE_STANDBY);
  indicators_clear();
  k_ops[next].entry();
}

static void state_update_warnings(void) {
  static bool a_little_hot;

  if (fans_pump_low()) {
    indicator_fast_flash(&g_indicators.low_pump_speed, COLOR_RED);
  } else if (g_state != STATE_INIT) {
    indicator_off(&g_indicators.low_pump_speed);
  }

  if (fans_fan_low()) {
    indicator_fast_flash(&g_indicators.low_fan_speed, COLOR_RED);
  } else if (g_state != STATE_INIT) {
    indicator_off(&g_indicators.low_fan_speed);
  }

  if (g_state == STATE_INIT) {
    return;
  }

  const float t = analog_control_c();
  const float hot = analog_ambient_c() + A_LITTLE_HOT_ABOVE_AIR_C;

  if (t > hot + A_LITTLE_HOT_HISTERESIS_C) {
    a_little_hot = true;
  } else if (t < hot - A_LITTLE_HOT_HISTERESIS_C) {
    a_little_hot = false;
  }

  if (a_little_hot) {
    indicator_flash(&g_indicators.a_little_hot, COLOR_RED);
  } else {
    indicator_off(&g_indicators.a_little_hot);
  }
}

void state_task(void *pvParameters) {
  (void)pvParameters;

  gpio_init(LOAD_EN_PIN);
  gpio_put(LOAD_EN_PIN, 0);
  gpio_set_dir(LOAD_EN_PIN, GPIO_OUT);
  usb_vbus_init();
  speaker_init();
  speaker_beep();

  state_goto(STATE_INIT);

  while (true) {
    k_ops[g_state].tick(xTaskGetTickCount());
    state_update_warnings();
    speaker_tick(g_state != STATE_INIT && indicators_flashing_red());
    vTaskDelay(pdMS_TO_TICKS(25));
  }
}
