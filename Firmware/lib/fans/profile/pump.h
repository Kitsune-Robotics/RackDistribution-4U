#pragma once

#include "curve.h"

/* not really tuned, stays flat till it's hot */
#define FAN_PROFILE_PUMP \
  FAN_CH(FAN_PUMP, 800,  \
         {0.0f, 180},    \
         {2.0f, 220},    \
         {4.0f, 240},   \
         {6.0f, 255},   \
         {8.0f, 255})
