#pragma once

#include <stdint.h>

typedef struct {
  float c;
  uint8_t duty;
} fan_curve_pt_t;

typedef struct {
  const fan_curve_pt_t *pts;
  unsigned n;
} fan_curve_ref_t;

#define FAN_CURVE_LEN(a) ((unsigned)(sizeof(a) / sizeof((a)[0])))
#define FAN_CURVE_REF(a) \
  { (a), FAN_CURVE_LEN(a) }

// Way to lookup any duty cycle for a given temp, interpolating between points
static inline uint8_t fan_curve_lookup(const fan_curve_pt_t *pts, unsigned n,
                                       float t) {
  if (!pts || n == 0) {
    return 0;
  }
  if (t <= pts[0].c) {
    return pts[0].duty;
  }
  if (t >= pts[n - 1].c) {
    return pts[n - 1].duty;
  }
  for (unsigned i = 1; i < n; i++) {
    if (t <= pts[i].c) {
      float span = pts[i].c - pts[i - 1].c;
      float u = span > 0.0f ? (t - pts[i - 1].c) / span : 0.0f;
      float d = (float)pts[i - 1].duty +
                u * (float)((int)pts[i].duty - (int)pts[i - 1].duty);
      if (d < 0.0f) {
        d = 0.0f;
      }
      if (d > 255.0f) {
        d = 255.0f;
      }
      return (uint8_t)(d + 0.5f);
    }
  }
  return pts[n - 1].duty;
}

static inline uint8_t fan_curve_ref_lookup(const fan_curve_ref_t *curve,
                                           float t) {
  if (!curve) {
    return 0;
  }
  return fan_curve_lookup(curve->pts, curve->n, t);
}
