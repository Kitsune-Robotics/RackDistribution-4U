#include "FreeRTOS.h"
#include "analog.h"
#include "aquacomputer.h"
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

  neopixel_ws2812_max_brightness = 64;
  neopixel_ws2812_init(&strip, pio0, NEOPIXEL_PIN, NEOPIXEL_FREQ_HZ, false,
                       NEOPIXEL_NUM_PIXELS);

  while (true) {
    TickType_t now = xTaskGetTickCount();
    indicators_flush(&strip, (uint32_t)now);
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

  // For the aquacomputer comms
  if (xTaskCreate(aquacomputer_task, "aqc", 512, NULL, 2, NULL) != pdPASS) {
    vApplicationMallocFailedHook();
  }

  vTaskStartScheduler();
  vApplicationMallocFailedHook();

  while (true) {
  }
}
