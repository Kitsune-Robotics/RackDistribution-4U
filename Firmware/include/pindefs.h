#pragma once

// Fan PWM / tach (GPIO)
#define PWM_7_PIN 0
#define TACH_7_PIN 1
#define PWM_6_PIN 2
#define TACH_6_PIN 3
#define PWM_5_PIN 4
#define TACH_5_PIN 5
#define PWM_4_PIN 6
#define TACH_4_PIN 7
#define PWM_3_PIN 8
#define TACH_3_PIN 9
#define PWM_2_PIN 10
#define TACH_2_PIN 11
#define PWM_1_PIN 12
#define TACH_1_PIN 13
#define PWM_0_PIN 14
#define TACH_0_PIN 15

// Panel
#define NEOPIXEL_PIN 16
#define NEOPIXEL_NUM_PIXELS 16
#define NEOPIXEL_FREQ_HZ 800000.0f
#define AC_LEDS_PIN 17

// Switches
#define SW_0_PIN 18
#define SW_1_PIN 19
#define SW_2_PIN 20

#define LOAD_EN_PIN 21
#define PCSPKR_PIN 22

// Temperature (GPIO / ADC channel)
#define TSENSOR_0_PIN 26
#define TSENSOR_0_ADC_CH 0
#define TSENSOR_1_PIN 27
#define TSENSOR_1_ADC_CH 1
#define TSENSOR_2_PIN 28
#define TSENSOR_2_ADC_CH 2

#define TSENSOR_PULLUP_OHMS 10000.0f
#define NTC_R25_OHMS 10000.0f
#define NTC_BETA 3950.0f
