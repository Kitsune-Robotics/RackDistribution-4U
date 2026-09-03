#pragma once

#include "curve.h"
#include "profile/pump.h"
#include "profile/rad.h"

#include "fans.h"

// Which curve each channel uses
static const fan_curve_ref_t k_fan_profile[FAN_COUNT] = {
    FAN_CURVE_REF(fan_profile_pump), // 0
    FAN_CURVE_REF(fan_profile_rad),  // 1
    FAN_CURVE_REF(fan_profile_rad),  // 2
    FAN_CURVE_REF(fan_profile_rad),  // 3
    FAN_CURVE_REF(fan_profile_rad),  // 4
    FAN_CURVE_REF(fan_profile_rad),  // 5
    FAN_CURVE_REF(fan_profile_rad),  // 6
    FAN_CURVE_REF(fan_profile_rad),  // 7
};
