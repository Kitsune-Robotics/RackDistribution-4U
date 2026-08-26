#include "FreeRTOS.h"
#include "indicators.h"
#include "neopixel_ws2812.h"
#include "pico/stdio.h"
#include "pindefs.h"
#include "task.h"

static void led_task(void *pvParameters) {
  (void)pvParameters;

  static neopixel_ws2812_t strip;
  static indicators_t indicators;

  neopixel_ws2812_max_brightness = 64;
  neopixel_ws2812_init(&strip, pio0, NEOPIXEL_PIN, NEOPIXEL_FREQ_HZ, false,
                       NEOPIXEL_NUM_PIXELS);

  indicators_clear(&indicators);
  indicator_solid(&indicators.standby, COLOR_YELLOW);
  indicator_off(&indicators.cooldown);
  indicator_flash(&indicators.a_little_hot, COLOR_RED);
  indicator_solid(&indicators.control, COLOR_GREEN);
  indicator_off(&indicators.low_coolant);
  indicator_off(&indicators.low_pump_speed);
  indicator_off(&indicators.low_flow);
  indicator_off(&indicators.low_fan_speed);

  while (true) {
    indicators_flush(&indicators, &strip, (uint32_t)xTaskGetTickCount());
    vTaskDelay(pdMS_TO_TICKS(25));
  }
}

int main(void) {
  stdio_init_all();

  if (xTaskCreate(led_task, "led", 512, NULL, 1, NULL) != pdPASS) {
    vApplicationMallocFailedHook();
  }

  vTaskStartScheduler();
  vApplicationMallocFailedHook();

  while (true) {
  }
}
