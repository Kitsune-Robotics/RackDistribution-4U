#pragma once

#include "curve.h"

// Not really tuned! Mostly stays flat till we start getting crazy hot.
static const fan_curve_pt_t fan_profile_pump[] = {
    {20.0f, 100},
    {25.0f, 100},
    {30.0f, 100},
    {35.0f, 100},
    {40.0f, 255},
};
