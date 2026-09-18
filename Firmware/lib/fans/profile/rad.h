#pragma once

#include "curve.h"

/* 200mm */
#define FAN_PROFILE_RAD \
  FAN_CH(FAN_RAD, 100,  \
         {0.0f, 40},    \
         {2.0f, 60},    \
         {4.0f, 80},    \
         {6.0f, 120},   \
         {8.0f, 160},   \
         {10.0f, 200},  \
         {12.0f, 255},  \
         {14.0f, 255},  \
         {16.0f, 255})
