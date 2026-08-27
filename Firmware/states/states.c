#include "state_internal.h"

#include "analog.h"
#include "indicators.h"
#include "parameters.h"

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
  indicators_clear();
  k_ops[next].entry();
}

static void state_update_warnings(void) {
  if (analog_tsensor1_c() > A_LITTLE_HOT_C) {
    indicator_flash(&g_indicators.a_little_hot, COLOR_RED);
  } else {
    indicator_off(&g_indicators.a_little_hot);
  }
}

void state_task(void *pvParameters) {
  (void)pvParameters;

  state_goto(STATE_INIT);

  while (true) {
    k_ops[g_state].tick(xTaskGetTickCount());
    if (g_state != STATE_INIT) {
      state_update_warnings();
    }
    vTaskDelay(pdMS_TO_TICKS(25));
  }
}
