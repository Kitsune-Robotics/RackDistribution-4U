#pragma once

#include "curve.h"

/* 200mm */
#define FAN_PROFILE_RAD \
  FAN_CH(FAN_RAD, 100,  \
         {0.0f, 20},    \
         {2.0f, 40},    \
         {4.0f, 60},    \
         {6.0f, 80},    \
         {8.0f, 100},   \
         {10.0f, 150},  \
         {12.0f, 180},  \
         {14.0f, 210},  \
         {16.0f, 230},  \
         {18.0f, 255})
