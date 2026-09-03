#pragma once

#include <stdint.h>

#define FAN_COUNT 8

void fans_task(void *pvParameters);

uint8_t fans_pwm_get(unsigned ch);
uint16_t fans_rpm(unsigned ch);
