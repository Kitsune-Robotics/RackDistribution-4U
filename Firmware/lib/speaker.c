#include "speaker.h"

#include "FreeRTOS.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "parameters.h"
#include "pindefs.h"
#include "switches.h"
#include "task.h"

static uint16_t g_half;

static void speaker_tone(bool on) {
  pwm_set_gpio_level(PCSPKR_PIN, on ? g_half : 0);
}

void speaker_init(void) {
  gpio_set_function(PCSPKR_PIN, GPIO_FUNC_PWM);

  uint slice = pwm_gpio_to_slice_num(PCSPKR_PIN);
  const uint16_t wrap = 249;
  float div = (float)clock_get_hz(clk_sys) / (PCSPKR_HZ * (float)(wrap + 1u));

  pwm_config cfg = pwm_get_default_config();
  pwm_config_set_clkdiv(&cfg, div);
  pwm_config_set_wrap(&cfg, wrap);
  pwm_init(slice, &cfg, true);

  g_half = (uint16_t)((wrap + 1u) / 2u);
  speaker_tone(false);
}

void speaker_beep(void) {
  if (switches_mute()) {
    return;
  }
  speaker_tone(true);
  vTaskDelay(pdMS_TO_TICKS(PCSPKR_BEEP_MS));
  speaker_tone(false);
}

void speaker_tick(bool alarm) {
  if (switches_mute() || !alarm) {
    speaker_tone(false);
    return;
  }

  TickType_t beep = pdMS_TO_TICKS(PCSPKR_BEEP_MS);
  TickType_t gap = pdMS_TO_TICKS(PCSPKR_GAP_MS);
  TickType_t phase = xTaskGetTickCount() % pdMS_TO_TICKS(PCSPKR_PERIOD_MS);
  bool on = phase < beep ||
            (phase >= beep + gap && phase < beep + gap + beep);
  speaker_tone(on);
}
