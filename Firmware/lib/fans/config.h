#pragma once

#include "profile/pump.h"
#include "profile/rad.h"

// Channel map (1-based headers): 1-2 unused, 3-4 rad, 5 unused, 6-7 rad, 8 pump
static const fan_ch_t k_fans[FAN_COUNT] = {
    [0] = FAN_NONE,
    [1] = FAN_NONE,
    [2] = FAN_PROFILE_RAD,
    [3] = FAN_PROFILE_RAD,
    [4] = FAN_NONE,
    [5] = FAN_PROFILE_RAD,
    [6] = FAN_PROFILE_RAD,
    [7] = FAN_PROFILE_PUMP,
};
