#pragma once

#include "curve.h"

/* 200mm */
#define FAN_PROFILE_RAD \
  FAN_CH(FAN_RAD, 100,  \
         {0.0f, 40},    \
         {2.0f, 80},    \
         {4.0f, 160},   \
         {6.0f, 255},  \
         {8.0f, 255})
