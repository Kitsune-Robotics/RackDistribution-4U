#pragma once

// Fans
#define FAN_PWM_HZ 25000.0f
#define FAN_TACH_PPR 2
#define FAN_RPM_WINDOW_MS 200

// Temperature
#define A_LITTLE_HOT_ABOVE_AIR_C 8.0f
#define A_LITTLE_HOT_HISTERESIS_C 0.25f
#define COOLDOWN_COLD_ABOVE_AIR_C 6.0f
#define TSENSOR0_OFFSET_C 1.9f /* coolant */
#define TSENSOR1_OFFSET_C -1.9f /* air */
#define TSENSOR2_OFFSET_C 0.0f /* exhaust */
#define AMBIENT_FALLBACK_C 20.0f

// PC speaker
#define PCSPKR_HZ 2000.0f
#define PCSPKR_BEEP_MS 80
#define PCSPKR_GAP_MS 80
#define PCSPKR_PERIOD_MS 1000

// 1 to enable
#define ENABLE_UF2_LOADER 0

// State configs
#define STATE_INIT_MS 4000
#define STATE_USB_LOST_MS 5000
#define STATE_COOLDOWN_COLD_MS 10000
