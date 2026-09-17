#pragma once

#include "curve.h"

/* not really tuned, stays flat till it's hot */
#define FAN_PROFILE_PUMP \
  FAN_CH(FAN_PUMP, 800,  \
         {0.0f, 160},    \
         {5.0f, 175},    \
         {10.0f, 190},   \
         {15.0f, 220},   \
         {20.0f, 255})
