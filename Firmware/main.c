#include "FreeRTOS.h"
#include "analog.h"
#include "indicators.h"
#include "neopixel_ws2812.h"
#include "pico/stdio.h"
#include "pindefs.h"
#include "task.h"

#define LED_STARTUP_MS 4000

// Boot lamp test
static void indicators_startup(indicators_t *inds) {
  indicator_flash(&inds->standby, COLOR_WHITE);
  indicator_solid(&inds->cooldown, COLOR_YELLOW);
  indicator_solid(&inds->a_little_hot, COLOR_RED);
  indicator_solid(&inds->control, COLOR_GREEN);
  indicator_solid(&inds->low_coolant, COLOR_RED);
  indicator_solid(&inds->low_pump_speed, COLOR_RED);
  indicator_solid(&inds->low_flow, COLOR_RED);
  indicator_solid(&inds->low_fan_speed, COLOR_RED);
}

static void indicators_normal(indicators_t *inds) {
  indicator_solid(&inds->standby, COLOR_YELLOW);
  indicator_off(&inds->cooldown);
  if (analog_tsensor1_c() > A_LITTLE_HOT_C) {
    indicator_flash(&inds->a_little_hot, COLOR_RED);
  } else {
    indicator_off(&inds->a_little_hot);
  }
  indicator_solid(&inds->control, COLOR_GREEN);
  indicator_off(&inds->low_coolant);
  indicator_off(&inds->low_pump_speed);
  indicator_off(&inds->low_flow);
  indicator_off(&inds->low_fan_speed);
}

static void led_task(void *pvParameters) {
  (void)pvParameters;

  static neopixel_ws2812_t strip;
  static indicators_t indicators;

  neopixel_ws2812_max_brightness = 64;
  neopixel_ws2812_init(&strip, pio0, NEOPIXEL_PIN, NEOPIXEL_FREQ_HZ, false,
                       NEOPIXEL_NUM_PIXELS);

  indicators_clear(&indicators);
  TickType_t boot = xTaskGetTickCount();

  while (true) {
    TickType_t now = xTaskGetTickCount();
    if ((now - boot) < pdMS_TO_TICKS(LED_STARTUP_MS)) {
      indicators_startup(&indicators);
    } else {
      indicators_normal(&indicators);
    }

    indicators_flush(&indicators, &strip, (uint32_t)now);
    vTaskDelay(pdMS_TO_TICKS(25));
  }
}

int main(void) {
  stdio_init_all();

  if (xTaskCreate(led_task, "led", 512, NULL, 1, NULL) != pdPASS) {
    vApplicationMallocFailedHook();
  }
  if (xTaskCreate(analog_task, "analog", 512, NULL, 2, NULL) != pdPASS) {
    vApplicationMallocFailedHook();
  }

  vTaskStartScheduler();
  vApplicationMallocFailedHook();

  while (true) {
  }
}
