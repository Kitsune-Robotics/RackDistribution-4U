#pragma once

#include "curve.h"

/* not really tuned, stays flat till it's hot */ 
#define FAN_PROFILE_PUMP \
  FAN_CH(FAN_PUMP, 800,  \
         {20.0f, 160},   \
         {25.0f, 175},   \
         {30.0f, 190},   \
         {35.0f, 220},   \
         {40.0f, 255})
