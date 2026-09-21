#include "fans.h"

#include "FreeRTOS.h"
#include "analog.h"
#include "config.h"
#include "internal.h"
#include "parameters.h"
#include "switches.h"
#include "task.h"

#include <stdio.h>

uint8_t g_duty[FAN_COUNT];

uint8_t fans_pwm_get(unsigned ch) {
  return ch < FAN_COUNT ? g_duty[ch] : 0;
}

static void curves_apply(void) {
  if (switches_max_cool()) {
    for (unsigned i = 0; i < FAN_COUNT; i++) {
      g_duty[i] = 255;
    }
    return;
  }
  float dc = analog_control_c() - analog_ambient_c();
  for (unsigned i = 0; i < FAN_COUNT; i++) {
    g_duty[i] = fan_duty_at(&k_fans[i], dc);
  }
}

void fans_task(void *pvParameters) {
  (void)pvParameters;

  curves_apply();
  pwm_init_all();
  tach_init();

  while (true) {
    curves_apply();
    pwm_wave_rebuild();
    rpm_update();
    printf("cool=%.1f  rpm %u %u %u %u  %u %u %u %u  pwm %u %u %u %u  %u %u %u %u\n",
           (double)analog_control_c(), fans_rpm(0), fans_rpm(1), fans_rpm(2),
           fans_rpm(3), fans_rpm(4), fans_rpm(5), fans_rpm(6), fans_rpm(7),
           g_duty[0], g_duty[1], g_duty[2], g_duty[3], g_duty[4], g_duty[5],
           g_duty[6], g_duty[7]);
    vTaskDelay(pdMS_TO_TICKS(FAN_RPM_WINDOW_MS));
  }
}
