#pragma once

#include <stdbool.h>

bool msc_uf2_ready_to_apply(void);
void msc_uf2_apply(void) __attribute__((noreturn));
