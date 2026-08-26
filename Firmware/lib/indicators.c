#include "indicators.h"
#include "neopixel_ws2812.h"
#include "pindefs.h"

#include <stdbool.h>
#include <string.h>

_Static_assert(INDICATOR_COUNT * LEDS_PER_INDICATOR == NEOPIXEL_NUM_PIXELS,
               "8 indicators * 2 leds != pixel count");

void indicator_set(indicator_t *ind, indicator_mode_t mode, led_color_t color) {
  if (!ind) {
    return;
  }
  ind->mode = mode;
  ind->color = (mode == INDICATOR_OFF) ? COLOR_OFF : color;
}

void indicators_clear(indicators_t *inds) {
  if (!inds) {
    return;
  }
  memset(inds, 0, sizeof(*inds));
}

static bool indicator_lit(indicator_mode_t mode, uint32_t now_ms) {
  switch (mode) {
  case INDICATOR_SOLID:
    return true;
  case INDICATOR_FLASH:
    return ((now_ms / INDICATOR_FLASH_MS) & 1u) == 0;
  case INDICATOR_FAST_FLASH:
    return ((now_ms / INDICATOR_FAST_FLASH_MS) & 1u) == 0;
  case INDICATOR_OFF:
  default:
    return false;
  }
}

void indicators_flush(const indicators_t *inds, neopixel_ws2812_t *strip,
                      uint32_t now_ms) {
  if (!inds || !strip) {
    return;
  }

  for (unsigned i = 0; i < INDICATOR_COUNT; i++) {
    const indicator_t *ind = &inds->by_index[i];
    const bool on = indicator_lit(ind->mode, now_ms);
    const uint8_t r = on ? ind->color.r : 0;
    const uint8_t g = on ? ind->color.g : 0;
    const uint8_t b = on ? ind->color.b : 0;

    for (unsigned led = 0; led < LEDS_PER_INDICATOR; led++) {
      neopixel_ws2812_set_pixel_rgb(strip, i * LEDS_PER_INDICATOR + led, r, g,
                                    b);
    }
  }
  neopixel_ws2812_flush(strip);
}
