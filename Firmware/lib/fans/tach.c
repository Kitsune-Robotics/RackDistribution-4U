#include "internal.h"

#include "config.h"
#include "fans.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "parameters.h"
#include "pindefs.h"
#include "states.h"

static const uint k_tach_pins[FAN_COUNT] = {
    TACH_0_PIN, TACH_1_PIN, TACH_2_PIN, TACH_3_PIN,
    TACH_4_PIN, TACH_5_PIN, TACH_6_PIN, TACH_7_PIN,
};

static volatile uint32_t g_edge_us[FAN_COUNT];
static volatile uint32_t g_period_us[FAN_COUNT];
static uint16_t g_rpm[FAN_COUNT];

static bool g_rpm_ready;
static bool g_pump_low;
static bool g_fan_low;
static unsigned g_pump_low_n;
static unsigned g_fan_low_n;

uint16_t fans_rpm(unsigned ch) { return ch < FAN_COUNT ? g_rpm[ch] : 0; }

bool fans_pump_low(void) { return g_rpm_ready && g_pump_low; }

bool fans_fan_low(void) { return g_rpm_ready && g_fan_low; }

static void tach_irq(uint gpio, uint32_t events) {
  (void)events;
  if (gpio < TACH_7_PIN || gpio > TACH_0_PIN || !(gpio & 1u)) {
    return;
  }
  unsigned i = (TACH_0_PIN - gpio) / 2u;
  uint32_t now = time_us_32();
  if (g_edge_us[i]) {
    g_period_us[i] = now - g_edge_us[i];
  }
  g_edge_us[i] = now;
}

void tach_init(void) {
  for (unsigned i = 0; i < FAN_COUNT; i++) {
    uint pin = k_tach_pins[i];
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
    gpio_set_input_hysteresis_enabled(pin, true);
    gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_RISE, true, tach_irq);
  }
}

void rpm_update(void) {
  uint32_t now = time_us_32();
  uint32_t stall = FAN_RPM_STALL_MS * 1000u;

  for (unsigned i = 0; i < FAN_COUNT; i++) {
    uint32_t edge = g_edge_us[i];
    uint32_t period = g_period_us[i];
    if (!edge || !period || period > stall || (now - edge) > stall) {
      g_rpm[i] = 0;
      continue;
    }
    uint32_t rpm = 60000000u / (period * FAN_TACH_PPR);
    g_rpm[i] = rpm > 0xffffu ? 0xffffu : (uint16_t)rpm;
  }

  bool pump_low = false;
  bool fan_low = false;
  if (state_get() != STATE_STANDBY) {
    for (unsigned i = 0; i < FAN_COUNT; i++) {
      const fan_ch_t *ch = &k_fans[i];
      if (i == 5 || ch->kind == FAN_NONE || g_duty[i] == 0) {
        continue; // 5: dead tach TODO: REMOVE ME
      }
      if (g_rpm[i] < ch->low_rpm) {
        if (ch->kind == FAN_PUMP) {
          pump_low = true;
        } else {
          fan_low = true;
        }
      }
    }
  }

  unsigned hold = FAN_RPM_LOW_MS / FAN_RPM_WINDOW_MS;
  if (pump_low) {
    if (g_pump_low_n < hold) {
      g_pump_low_n++;
    }
  } else {
    g_pump_low_n = 0;
  }
  if (fan_low) {
    if (g_fan_low_n < hold) {
      g_fan_low_n++;
    }
  } else {
    g_fan_low_n = 0;
  }
  g_pump_low = g_pump_low_n >= hold;
  g_fan_low = g_fan_low_n >= hold;
  g_rpm_ready = true;
}
