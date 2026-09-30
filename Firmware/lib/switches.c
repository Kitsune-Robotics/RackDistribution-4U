#include "switches.h"

#include "hardware/gpio.h"
#include "pindefs.h"

#define SW_DEBOUNCE 2

static const uint k_pins[] = {SW_INHIBIT_PIN, SW_MAX_COOL_PIN, SW_MUTE_PIN};
static uint8_t g_count[3];
static uint8_t g_on;

void switches_init(void) {
  for (unsigned i = 0; i < 3; i++) {
    gpio_init(k_pins[i]);
    gpio_set_dir(k_pins[i], GPIO_IN);
    gpio_pull_up(k_pins[i]);
    gpio_set_input_hysteresis_enabled(k_pins[i], true);
    if (!gpio_get(k_pins[i])) {
      g_count[i] = SW_DEBOUNCE;
      g_on |= (uint8_t)(1u << i);
    }
  }
}

void switches_tick(void) {
  for (unsigned i = 0; i < 3; i++) {
    if (!gpio_get(k_pins[i])) {
      if (g_count[i] < SW_DEBOUNCE) {
        g_count[i]++;
      }
      if (g_count[i] >= SW_DEBOUNCE) {
        g_on |= (uint8_t)(1u << i);
      }
    } else {
      if (g_count[i] > 0) {
        g_count[i]--;
      }
      if (g_count[i] == 0) {
        g_on &= (uint8_t)~(1u << i);
      }
    }
  }
}

bool switches_inhibit(void) { return (g_on & 1u) != 0; }

bool switches_max_cool(void) { return (g_on & 2u) != 0; }

bool switches_mute(void) { return (g_on & 4u) != 0; }
