#pragma once

#include <stdbool.h>
#include <stdint.h>

void fans_task(void *pvParameters);

uint8_t fans_pwm_get(unsigned ch);
uint16_t fans_rpm(unsigned ch);
bool fans_pump_low(void);
bool fans_fan_low(void);
