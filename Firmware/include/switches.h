#pragma once

#include <stdbool.h>

void switches_init(void);
void switches_tick(void);

bool switches_inhibit(void);
bool switches_max_cool(void);
bool switches_mute(void);
