#pragma once

#include "curve.h"

// Tuned for the 200mm fans
static const fan_curve_pt_t fan_profile_rad[] = {
    {20.0f, 20},
    {22.0f, 40},
    {24.0f, 60},
    {26.0f, 80},
    {28.0f, 100},
    {30.0f, 150},
    {32.0f, 180},
    {34.0f, 210},
    {36.0f, 230},
    {38.0f, 255},
};
