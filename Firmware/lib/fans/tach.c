#include "internal.h"

#include "aquacomputer.h"
#include "config.h"
#include "fans.h"
#include "hardware/gpio.h"
#include "parameters.h"
#include "pindefs.h"
#include "states.h"

static const uint k_tach_pins[FAN_COUNT] = {
    TACH_0_PIN, TACH_1_PIN, TACH_2_PIN, TACH_3_PIN,
    TACH_4_PIN, TACH_5_PIN, TACH_6_PIN, TACH_7_PIN,
};

static volatile uint32_t g_pulses[FAN_COUNT];
static uint32_t g_pulses_last[FAN_COUNT];
static uint16_t g_rpm[FAN_COUNT];

// First RPM sample waits one tach window so boot-zero doesn't trip lamps.
static bool g_rpm_ready;
static bool g_pump_low;
static bool g_fan_low;

uint16_t fans_rpm(unsigned ch) { return ch < FAN_COUNT ? g_rpm[ch] : 0; }

bool fans_pump_low(void) { return g_rpm_ready && g_pump_low; }

bool fans_fan_low(void) { return g_rpm_ready && g_fan_low; }

static void tach_irq(uint gpio, uint32_t events) {
  (void)events;
  if (gpio >= TACH_7_PIN && gpio <= TACH_0_PIN && (gpio & 1u)) {
    g_pulses[(TACH_0_PIN - gpio) / 2u]++;
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
  for (unsigned i = 0; i < FAN_COUNT; i++) {
    uint32_t n = g_pulses[i] - g_pulses_last[i];
    g_pulses_last[i] = g_pulses[i];
    uint32_t rpm =
        (uint32_t)((uint64_t)n * 60u * 1000u /
                   (uint32_t)(FAN_TACH_PPR * FAN_RPM_WINDOW_MS));
    if (rpm > 0xffffu) {
      rpm = 0xffffu;
    }
    g_rpm[i] = (uint16_t)rpm;
  }

  bool pump_low = false;
  bool fan_low = false;
  if (state_get() != STATE_STANDBY) {
    for (unsigned i = 0; i < FAN_COUNT; i++) {
      const fan_ch_t *ch = &k_fans[i];
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

  for (unsigned i = 0; i < AQC_NUM_FANS; i++) {
    aquacomputer_set_fan_rpm(i, g_rpm[i]);
  }
}
