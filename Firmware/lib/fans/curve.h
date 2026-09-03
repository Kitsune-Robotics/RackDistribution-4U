#pragma once

#include <stdint.h>

#define FAN_COUNT 8
#define FAN_CURVE_MAX 16

typedef enum {
  FAN_NONE = 0,
  FAN_PUMP,
  FAN_RAD,
} fan_kind_t;

typedef struct {
  float c;
  uint8_t duty;
} fan_pt_t;

typedef struct {
  fan_kind_t kind;
  uint16_t low_rpm;
  uint8_t n;
  fan_pt_t curve[FAN_CURVE_MAX];
} fan_ch_t;

#define FAN_CH(kind_, low_, ...)                                               \
  {                                                                            \
    .kind = (kind_), .low_rpm = (low_),                                        \
    .n = (uint8_t)(sizeof((fan_pt_t[]){__VA_ARGS__}) / sizeof(fan_pt_t)),      \
    .curve = {__VA_ARGS__}                                                     \
  }

static inline uint8_t fan_duty_at(const fan_ch_t *ch, float t) {
  if (!ch || ch->kind == FAN_NONE || ch->n == 0) {
    return 0;
  }
  const fan_pt_t *p = ch->curve;
  unsigned n = ch->n;
  if (t <= p[0].c) {
    return p[0].duty;
  }
  if (t >= p[n - 1].c) {
    return p[n - 1].duty;
  }
  for (unsigned i = 1; i < n; i++) {
    if (t <= p[i].c) {
      float span = p[i].c - p[i - 1].c;
      float u = span > 0.0f ? (t - p[i - 1].c) / span : 0.0f;
      float d = (float)p[i - 1].duty +
                u * (float)((int)p[i].duty - (int)p[i - 1].duty);
      if (d < 0.0f) {
        d = 0.0f;
      }
      if (d > 255.0f) {
        d = 255.0f;
      }
      return (uint8_t)(d + 0.5f);
    }
  }
  return p[n - 1].duty;
}
