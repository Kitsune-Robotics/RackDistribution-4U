#include "fans.h"

#include "FreeRTOS.h"
#include "aquacomputer.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "parameters.h"
#include "pindefs.h"
#include "states.h"
#include "task.h"

#include "od_pwm.pio.h"

#include <stdio.h>

#define PWM_PIN_MASK 0x5555u
#define PWM_WAVE_STEPS 256

static const uint k_pwm_pins[FAN_COUNT] = {
    PWM_0_PIN, PWM_1_PIN, PWM_2_PIN, PWM_3_PIN,
    PWM_4_PIN, PWM_5_PIN, PWM_6_PIN, PWM_7_PIN,
};

static const uint k_tach_pins[FAN_COUNT] = {
    TACH_0_PIN, TACH_1_PIN, TACH_2_PIN, TACH_3_PIN,
    TACH_4_PIN, TACH_5_PIN, TACH_6_PIN, TACH_7_PIN,
};

static volatile uint32_t g_pulses[FAN_COUNT];
static uint32_t g_pulses_last[FAN_COUNT];
static uint16_t g_rpm[FAN_COUNT];
static uint8_t g_duty[FAN_COUNT];

static PIO g_pwm_pio;
static uint g_pwm_sm;
static uint g_pwm_dma;
static uint g_pwm_dma_ctrl;
static uint32_t g_pwm_dma_count = PWM_WAVE_STEPS;
static uint32_t g_pwm_wave[PWM_WAVE_STEPS] __attribute__((aligned(1024)));

static void pwm_wave_rebuild(void) {
  system_state_t st = state_get();
  bool on = (st == STATE_RUN || st == STATE_COOLDOWN);

  for (unsigned step = 0; step < PWM_WAVE_STEPS; step++) {
    uint32_t dirs = 0;
    for (unsigned ch = 0; ch < FAN_COUNT; ch++) {
      uint8_t duty = on ? g_duty[ch] : 0;
      uint16_t thresh =
          (uint16_t)(((uint32_t)duty * PWM_WAVE_STEPS + 127u) / 255u);
      uint16_t low_steps = (uint16_t)(PWM_WAVE_STEPS - thresh);
      if (step < low_steps) {
        dirs |= 1u << k_pwm_pins[ch];
      }
    }
    g_pwm_wave[step] = dirs;
  }
}

void fans_pwm_set(unsigned ch, uint8_t duty) {
  if (ch >= FAN_COUNT) {
    return;
  }
  g_duty[ch] = duty;
  pwm_wave_rebuild();
}

uint8_t fans_pwm_get(unsigned ch) {
  return ch < FAN_COUNT ? g_duty[ch] : 0;
}

uint16_t fans_rpm(unsigned ch) { return ch < FAN_COUNT ? g_rpm[ch] : 0; }

static void pwm_init_all(void) {
  g_pwm_pio = pio1;
  uint offset = pio_add_program(g_pwm_pio, &od_pwm_program);
  g_pwm_sm = pio_claim_unused_sm(g_pwm_pio, true);

  for (unsigned i = 0; i < FAN_COUNT; i++) {
    uint pin = k_pwm_pins[i];
    pio_gpio_init(g_pwm_pio, pin);
    gpio_disable_pulls(pin);
    gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_12MA);
  }

  pio_sm_config c = od_pwm_program_get_default_config(offset);
  sm_config_set_out_pins(&c, 0, 16);
  sm_config_set_out_shift(&c, true, true, 32);
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
  float div = (float)clock_get_hz(clk_sys) /
              (FAN_PWM_HZ * (float)PWM_WAVE_STEPS * 2.0f);
  sm_config_set_clkdiv(&c, div);

  pio_sm_init(g_pwm_pio, g_pwm_sm, offset, &c);
  pio_sm_set_pins_with_mask(g_pwm_pio, g_pwm_sm, 0, PWM_PIN_MASK);
  pio_sm_set_pindirs_with_mask(g_pwm_pio, g_pwm_sm, 0, PWM_PIN_MASK);

  pwm_wave_rebuild();

  g_pwm_dma = dma_claim_unused_channel(true);
  g_pwm_dma_ctrl = dma_claim_unused_channel(true);

  dma_channel_config dc = dma_channel_get_default_config(g_pwm_dma);
  channel_config_set_transfer_data_size(&dc, DMA_SIZE_32);
  channel_config_set_read_increment(&dc, true);
  channel_config_set_write_increment(&dc, false);
  channel_config_set_dreq(&dc, pio_get_dreq(g_pwm_pio, g_pwm_sm, true));
  channel_config_set_ring(&dc, false, 10);
  channel_config_set_chain_to(&dc, g_pwm_dma_ctrl);
  dma_channel_configure(g_pwm_dma, &dc, &g_pwm_pio->txf[g_pwm_sm], g_pwm_wave,
                        PWM_WAVE_STEPS, false);

  dma_channel_config cc = dma_channel_get_default_config(g_pwm_dma_ctrl);
  channel_config_set_transfer_data_size(&cc, DMA_SIZE_32);
  channel_config_set_read_increment(&cc, false);
  channel_config_set_write_increment(&cc, false);
  dma_channel_configure(g_pwm_dma_ctrl, &cc,
                        &dma_hw->ch[g_pwm_dma].al1_transfer_count_trig,
                        &g_pwm_dma_count, 1, false);

  dma_channel_start(g_pwm_dma);
  pio_sm_set_enabled(g_pwm_pio, g_pwm_sm, true);
}

static void tach_irq(uint gpio, uint32_t events) {
  (void)events;
  if (gpio >= TACH_7_PIN && gpio <= TACH_0_PIN && (gpio & 1u)) {
    g_pulses[(TACH_0_PIN - gpio) / 2u]++;
  }
}

static void tach_init(void) {
  for (unsigned i = 0; i < FAN_COUNT; i++) {
    uint pin = k_tach_pins[i];
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
    gpio_set_input_hysteresis_enabled(pin, true);
    gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_RISE, true, tach_irq);
  }
}

static void rpm_update(void) {
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

  for (unsigned i = 0; i < AQC_NUM_FANS; i++) {
    aquacomputer_set_fan_rpm(i, g_rpm[i]);
  }
}

static void pwm_follow_host(void) {
  for (unsigned i = 0; i < AQC_NUM_FANS; i++) {
    g_duty[i] = aquacomputer_pwm(i);
  }
  pwm_wave_rebuild();
}

void fans_task(void *pvParameters) {
  (void)pvParameters;

  pwm_init_all();
  tach_init();
  printf("fan0 PWM=GP%u (open-drain) TACH=GP%u (pull-up)\n", PWM_0_PIN,
         TACH_0_PIN);

  TickType_t last_rpm = xTaskGetTickCount();

  while (true) {
    pwm_follow_host();

    TickType_t now = xTaskGetTickCount();
    if ((now - last_rpm) >= pdMS_TO_TICKS(FAN_RPM_WINDOW_MS)) {
      rpm_update();
      last_rpm = now;
      printf("rpm %u %u %u %u  %u %u %u %u  pwm %u %u %u %u  p %u %u %u %u\n",
             g_rpm[0], g_rpm[1], g_rpm[2], g_rpm[3], g_rpm[4], g_rpm[5],
             g_rpm[6], g_rpm[7], g_duty[0], g_duty[1], g_duty[2], g_duty[3],
             (unsigned)g_pulses[0], (unsigned)g_pulses[1], (unsigned)g_pulses[2],
             (unsigned)g_pulses[3]);
    }

    vTaskDelay(pdMS_TO_TICKS(2));
  }
}
