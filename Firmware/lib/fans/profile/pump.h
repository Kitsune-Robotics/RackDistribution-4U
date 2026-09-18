#pragma once

#include "curve.h"

/* not really tuned, stays flat till it's hot */
#define FAN_PROFILE_PUMP \
  FAN_CH(FAN_PUMP, 800,  \
         {0.0f, 180},    \
         {5.0f, 180},    \
         {10.0f, 180},   \
         {12.0f, 255},   \
         {15.0f, 255})
