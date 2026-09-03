#pragma once

#include "curve.h"

/* not really tuned, stays flat till it's hot */ 
#define FAN_PROFILE_PUMP \
  FAN_CH(FAN_PUMP, 800,  \
         {20.0f, 100},   \
         {25.0f, 100},   \
         {30.0f, 100},   \
         {35.0f, 100},   \
         {40.0f, 255})
