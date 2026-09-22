#pragma once

#include "curve.h"

/* 200mm */
#define FAN_PROFILE_RAD \
  FAN_CH(FAN_RAD, 100,  \
         {0.0f, 0},    \
         {1.0f, 140},   \
         {2.0f, 200},   \
         {3.0f, 255},   \
         {5.0f, 255})
