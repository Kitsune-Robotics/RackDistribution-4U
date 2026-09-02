#pragma once

#include "neopixel_ws2812.h"
#include <stdint.h>

#define INDICATOR_COUNT 8
#define LEDS_PER_INDICATOR 2

#define INDICATOR_FLASH_MS 500
#define INDICATOR_FAST_FLASH_MS 125

typedef enum {
  INDICATOR_OFF = 0,
  INDICATOR_SOLID,
  INDICATOR_FLASH,
  INDICATOR_FAST_FLASH,
} indicator_mode_t;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} led_color_t;

#define COLOR_OFF ((led_color_t){0, 0, 0})
#define COLOR_RED ((led_color_t){255, 0, 0})
#define COLOR_GREEN ((led_color_t){0, 255, 0})
#define COLOR_BLUE ((led_color_t){0, 0, 255})
#define COLOR_YELLOW ((led_color_t){255, 255, 0})
#define COLOR_ORANGE ((led_color_t){255, 80, 0})
#define COLOR_CYAN ((led_color_t){0, 255, 255})
#define COLOR_MAGENTA ((led_color_t){255, 0, 255})
#define COLOR_WHITE ((led_color_t){255, 255, 255})

typedef struct {
  indicator_mode_t mode;
  led_color_t color;
} indicator_t;

// All the indicators in order
typedef union {
  struct {
    indicator_t standby;
    indicator_t cooldown;
    indicator_t a_little_hot;
    indicator_t control;
    indicator_t low_coolant;
    indicator_t low_pump_speed;
    indicator_t low_flow;
    indicator_t low_fan_speed;
  };
  indicator_t by_index[INDICATOR_COUNT];
} indicators_t;

extern indicators_t g_indicators;

void indicator_set(indicator_t *ind, indicator_mode_t mode, led_color_t color);
void indicators_clear(void);
void indicators_flush(neopixel_ws2812_t *strip, uint32_t now_ms);

static inline void indicator_solid(indicator_t *ind, led_color_t color) {
  indicator_set(ind, INDICATOR_SOLID, color);
}

static inline void indicator_flash(indicator_t *ind, led_color_t color) {
  indicator_set(ind, INDICATOR_FLASH, color);
}

static inline void indicator_fast_flash(indicator_t *ind, led_color_t color) {
  indicator_set(ind, INDICATOR_FAST_FLASH, color);
}

static inline void indicator_off(indicator_t *ind) {
  indicator_set(ind, INDICATOR_OFF, COLOR_OFF);
}
