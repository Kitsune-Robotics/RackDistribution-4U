#pragma once

#include "curve.h"

#include <stdint.h>

extern uint8_t g_duty[FAN_COUNT];

void pwm_init_all(void);
void pwm_wave_rebuild(void);

void tach_init(void);
void rpm_update(void);
