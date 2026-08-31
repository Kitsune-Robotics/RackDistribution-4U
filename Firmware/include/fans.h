#pragma once

#include <stdint.h>

#define FAN_COUNT 8

void fans_task(void *pvParameters);

void fans_pwm_set(unsigned ch, uint8_t duty);
uint8_t fans_pwm_get(unsigned ch);
uint16_t fans_rpm(unsigned ch);
