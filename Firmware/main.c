#include "FreeRTOS.h"
#include "analog.h"
#include "hid.h"
#include "fans.h"
#include "indicators.h"
#include "neopixel_ws2812.h"
#include "pico/stdio.h"
#include "pindefs.h"
#include "states.h"
#include "task.h"

static void led_task(void *pvParameters) {
  (void)pvParameters;

  static neopixel_ws2812_t strip;
  static neopixel_ws2812_t ac;

  neopixel_ws2812_max_brightness = 64;
  neopixel_ws2812_init(&strip, pio0, NEOPIXEL_PIN, NEOPIXEL_FREQ_HZ, false,
                       NEOPIXEL_NUM_PIXELS);
  neopixel_ws2812_init(&ac, pio0, AC_LEDS_PIN, NEOPIXEL_FREQ_HZ, false,
                       AC_LEDS_NUM_PIXELS);

  while (true) {
    TickType_t now = xTaskGetTickCount();
    indicators_flush(&strip, (uint32_t)now);
    if (state_get() == STATE_RUN) {
      neopixel_ws2812_put_rgb(&ac, 220, 220, 220);
    } else {
      neopixel_ws2812_put_rgb(&ac, 10, 0, 0);
    }
    vTaskDelay(pdMS_TO_TICKS(25));
  }
}

int main(void) {
  stdio_init_all();

  if (xTaskCreate(state_task, "state", 512, NULL, 3, NULL) != pdPASS) {
    vApplicationMallocFailedHook();
  }
  if (xTaskCreate(led_task, "led", 512, NULL, 1, NULL) != pdPASS) {
    vApplicationMallocFailedHook();
  }
  if (xTaskCreate(analog_task, "analog", 512, NULL, 2, NULL) != pdPASS) {
    vApplicationMallocFailedHook();
  }
  if (xTaskCreate(fans_task, "fans", 512, NULL, 2, NULL) != pdPASS) {
    vApplicationMallocFailedHook();
  }

  if (xTaskCreate(hid_task, "hid", 512, NULL, 2, NULL) != pdPASS) {
    vApplicationMallocFailedHook();
  }

  vTaskStartScheduler();
  vApplicationMallocFailedHook();

  while (true) {
  }
}
