#pragma once

float analog_coolant_c(void); /* TSENSOR 0 / ADC0, NAN if unplugged */
float analog_air_c(void);     /* TSENSOR 1 / ADC1, NAN if unplugged */
float analog_exhaust_c(void); /* TSENSOR 2 / ADC2, NAN if unplugged */
float analog_ambient_c(void); /* air, or AMBIENT_FALLBACK_C */
float analog_control_c(void); /* coolant, else ambient */
void analog_task(void *pvParameters);
