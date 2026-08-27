#pragma once

#include "FreeRTOS.h"
#include "states.h"
#include "task.h"

void state_goto(system_state_t next);
TickType_t state_entered(void);

void state_init_tick(TickType_t now);
void state_init_entry(void);

void state_standby_tick(TickType_t now);
void state_standby_entry(void);

void state_run_tick(TickType_t now);
void state_run_entry(void);

void state_cooldown_tick(TickType_t now);
void state_cooldown_entry(void);
