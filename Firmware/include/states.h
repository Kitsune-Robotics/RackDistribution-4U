#pragma once

typedef enum {
  STATE_INIT,
  STATE_STANDBY,
  STATE_COOLDOWN,
  STATE_RUN,
} system_state_t;

system_state_t state_get(void);
void state_task(void *pvParameters);
