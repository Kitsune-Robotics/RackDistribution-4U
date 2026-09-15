#include "internal.h"

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "od_pwm.pio.h"
#include "parameters.h"
#include "pindefs.h"
#include "states.h"

#include <stdbool.h>

#define PWM_PIN_MASK 0x5555u
#define PWM_WAVE_STEPS 256

static const uint k_pwm_pins[FAN_COUNT] = {
    PWM_0_PIN, PWM_1_PIN, PWM_2_PIN, PWM_3_PIN,
    PWM_4_PIN, PWM_5_PIN, PWM_6_PIN, PWM_7_PIN,
};

static PIO g_pwm_pio;
static uint g_pwm_sm;
static uint g_pwm_dma;
static uint g_pwm_dma_ctrl;
static uint32_t g_pwm_dma_count = PWM_WAVE_STEPS;
static uint32_t g_pwm_wave[PWM_WAVE_STEPS] __attribute__((aligned(1024)));

void pwm_wave_rebuild(void) {
  system_state_t st = state_get();
  bool on = (st != STATE_STANDBY);

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

void pwm_init_all(void) {
  g_pwm_pio = pio1; // PIO block 1
  uint offset = pio_add_program(g_pwm_pio, &od_pwm_program); // Add the program to the pio
  g_pwm_sm = pio_claim_unused_sm(g_pwm_pio, true); // Grab one of the state machines

  // For every fan...
  for (unsigned i = 0; i < FAN_COUNT; i++) {
    uint pin = k_pwm_pins[i];
    pio_gpio_init(g_pwm_pio, pin); // Initialize the GPIO pin
    gpio_disable_pulls(pin); // Disable the pull-ups
    gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_12MA); // Set the drive strength to 12mA
  }

  // ===========================
  // State machine configuration
  // ===========================
  pio_sm_config c = od_pwm_program_get_default_config(offset); // Get the default configuration

  // Set the output pins
  sm_config_set_out_pins(&c, 0, 16);
  sm_config_set_out_shift(&c, true, true, 32);
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

  // Set the clock division
  float div = (float)clock_get_hz(clk_sys) /
              (FAN_PWM_HZ * (float)PWM_WAVE_STEPS * 2.0f);
  sm_config_set_clkdiv(&c, div);

  // Initialize the state machine
  pio_sm_init(g_pwm_pio, g_pwm_sm, offset, &c);

  // Set the pins
  pio_sm_set_pins_with_mask(g_pwm_pio, g_pwm_sm, 0, PWM_PIN_MASK);
  pio_sm_set_pindirs_with_mask(g_pwm_pio, g_pwm_sm, 0, PWM_PIN_MASK);

  pwm_wave_rebuild();

  // ===========================
  // DMA configuration
  // ===========================
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

  // Start the state machine
  pio_sm_set_enabled(g_pwm_pio, g_pwm_sm, true);
}
