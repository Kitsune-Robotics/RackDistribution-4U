#pragma once

#include "curve.h"

/* 200mm */
#define FAN_PROFILE_RAD \
  FAN_CH(FAN_RAD, 100,  \
         {20.0f, 20},   \
         {22.0f, 40},   \
         {24.0f, 60},   \
         {26.0f, 80},   \
         {28.0f, 100},  \
         {30.0f, 150},  \
         {32.0f, 180},  \
         {34.0f, 210},  \
         {36.0f, 230},  \
         {38.0f, 255})
