#pragma once

#include "curve.h"

/* 200mm */
#define FAN_PROFILE_RAD \
  FAN_CH(FAN_RAD, 100,  \
         {0.0f, 20},    \
         {2.0f, 160},   \
         {4.0f, 255},   \
         {6.0f, 255},   \
         {8.0f, 255})
