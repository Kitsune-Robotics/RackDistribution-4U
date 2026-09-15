#pragma once

#include "profile/pump.h"
#include "profile/rad.h"

// Channel map!
static const fan_ch_t k_fans[FAN_COUNT] = {
    [0] = FAN_PROFILE_PUMP,
    [1] = FAN_PROFILE_RAD,
    [2] = FAN_PROFILE_RAD,
    [3] = FAN_PROFILE_RAD,
    [4] = FAN_PROFILE_RAD,
};
