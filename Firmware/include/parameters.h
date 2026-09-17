#pragma once

// Fans
#define FAN_PWM_HZ 25000.0f
#define FAN_TACH_PPR 2
#define FAN_RPM_WINDOW_MS 200

// Temperature
#define A_LITTLE_HOT_ABOVE_AIR_C 10.0f
#define A_LITTLE_HOT_HISTERESIS_C 0.25f
#define COOLDOWN_COLD_ABOVE_AIR_C 5.0f
#define TSENSOR0_OFFSET_C 1.9f /* coolant */
#define TSENSOR1_OFFSET_C -1.5f /* air */
#define TSENSOR2_OFFSET_C 0.0f /* exhaust */
#define AMBIENT_FALLBACK_C 20.0f

// 1 to enable
#define ENABLE_UF2_LOADER 1

// State configs
#define STATE_INIT_MS 4000
#define STATE_USB_LOST_MS 5000
#define STATE_COOLDOWN_COLD_MS 10000
