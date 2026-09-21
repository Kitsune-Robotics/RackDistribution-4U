#include "internal.h"

#include "config.h"
#include "fans.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/timer.h"
#include "parameters.h"
#include "pindefs.h"
#include "states.h"

static const uint k_tach_pins[FAN_COUNT] = {
    TACH_0_PIN, TACH_1_PIN, TACH_2_PIN, TACH_3_PIN,
    TACH_4_PIN, TACH_5_PIN, TACH_6_PIN, TACH_7_PIN,
};

static volatile uint32_t g_pulses[FAN_COUNT];
static volatile uint32_t g_last_edge_us[FAN_COUNT];
static volatile uint32_t g_period_us[FAN_COUNT];
static uint32_t g_pulses_last[FAN_COUNT];
static uint32_t g_span_edge_us[FAN_COUNT];
static uint16_t g_rpm[FAN_COUNT];

// First RPM sample waits one tach window so boot-zero doesn't trip lamps.
static bool g_rpm_ready;
static bool g_pump_low;
static bool g_fan_low;

uint16_t fans_rpm(unsigned ch) { return ch < FAN_COUNT ? g_rpm[ch] : 0; }

bool fans_pump_low(void) { return g_rpm_ready && g_pump_low; }

bool fans_fan_low(void) { return g_rpm_ready && g_fan_low; }

static uint16_t rpm_from_dt(uint32_t n, uint32_t dt_us) {
  if (n == 0 || dt_us == 0) {
    return 0;
  }
  uint32_t rpm = (uint32_t)((uint64_t)n * 60000000ull /
                            ((uint64_t)dt_us * FAN_TACH_PPR));
  if (rpm > 0xffffu) {
    rpm = 0xffffu;
  }
  return (uint16_t)rpm;
}

static void tach_irq(uint gpio, uint32_t events) {
  (void)events;
  if (gpio >= TACH_7_PIN && gpio <= TACH_0_PIN && (gpio & 1u)) {
    unsigned i = (TACH_0_PIN - gpio) / 2u;
    uint32_t now = time_us_32();
    uint32_t prev = g_last_edge_us[i];
    if (prev) {
      g_period_us[i] = now - prev;
    }
    g_last_edge_us[i] = now;
    g_pulses[i]++;
  }
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
  uint32_t pulses[FAN_COUNT];
  uint32_t edges[FAN_COUNT];
  uint32_t periods[FAN_COUNT];
  uint32_t now = time_us_32();
  const uint32_t stall_us = (uint32_t)FAN_RPM_STALL_MS * 1000u;

  uint32_t ints = save_and_disable_interrupts();
  for (unsigned i = 0; i < FAN_COUNT; i++) {
    pulses[i] = g_pulses[i];
    edges[i] = g_last_edge_us[i];
    periods[i] = g_period_us[i];
  }
  restore_interrupts(ints);

  for (unsigned i = 0; i < FAN_COUNT; i++) {
    uint32_t n = pulses[i] - g_pulses_last[i];
    g_pulses_last[i] = pulses[i];

    if (edges[i] == 0 || (now - edges[i]) > stall_us) {
      g_rpm[i] = 0;
      g_span_edge_us[i] = 0;
      continue;
    }
    if (n == 0) {
      continue;
    }

    uint16_t rpm = 0;
    if (g_span_edge_us[i] != 0) {
      uint32_t dt = edges[i] - g_span_edge_us[i];
      if (dt > 0 && dt <= stall_us) {
        rpm = rpm_from_dt(n, dt);
      }
    }
    g_span_edge_us[i] = edges[i];
    if (!rpm && periods[i]) {
      rpm = rpm_from_dt(1u, periods[i]);
    }
    if (rpm) {
      g_rpm[i] = rpm;
    }
  }

  bool pump_low = false;
  bool fan_low = false;
  if (state_get() != STATE_STANDBY) {
    for (unsigned i = 0; i < FAN_COUNT; i++) {
      const fan_ch_t *ch = &k_fans[i];
      if (i == 5u) {
        continue; /* Radiator 3 tach pin is dead */
        // TODO: remove this once i fix it
      }
      if (ch->kind != FAN_NONE && g_rpm[i] < ch->low_rpm) {
        if (ch->kind == FAN_PUMP) {
          pump_low = true;
        } else {
          fan_low = true;
        }
      }
    }
  }
  g_pump_low = pump_low;
  g_fan_low = fan_low;
  g_rpm_ready = true;
}
