#pragma once

#include "hardware/pio.h"
#include <stdbool.h>
#include <stdint.h>

// WS2812 over PIO. GRB. cmake turns ws2812.pio into a header.

#define NEOPIXEL_WS2812_MAX_PIXELS 50

typedef struct neopixel_ws2812 {
  PIO pio;
  uint sm;
  uint pin;
  bool is_rgbw;
  uint num_pixels;
  uint32_t pixel_buf[NEOPIXEL_WS2812_MAX_PIXELS];
} neopixel_ws2812_t;

extern uint8_t neopixel_ws2812_max_brightness;

void neopixel_ws2812_init(neopixel_ws2812_t *np, PIO pio, uint pin,
                          float freq_hz, bool is_rgbw, uint num_pixels);

void neopixel_ws2812_put_rgb(neopixel_ws2812_t *np, uint8_t r, uint8_t g,
                             uint8_t b);

void neopixel_ws2812_set_pixel_rgb(neopixel_ws2812_t *np, uint index,
                                  uint8_t r, uint8_t g, uint8_t b);

void neopixel_ws2812_flush(neopixel_ws2812_t *np);

void neopixel_ws2812_put_grb_u32(neopixel_ws2812_t *np, uint32_t grb);
