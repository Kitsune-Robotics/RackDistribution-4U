#pragma once

#include <stdint.h>

// Aquacomputer Quadro — what aquacomputer_d5next binds on.
#define AQC_USB_VID 0x0c70
#define AQC_USB_PID 0xf00d

#define AQC_STATUS_REPORT_ID 0x01
#define AQC_STATUS_REPORT_SIZE 0xa8

#define AQC_SAVE_REPORT_ID 0x02
#define AQC_SAVE_REPORT_SIZE 0x0b

#define AQC_CTRL_REPORT_ID 0x03
#define AQC_CTRL_REPORT_SIZE 0x3c1

#define AQC_NUM_TEMPS 4
#define AQC_NUM_FANS 4
#define AQC_SENSOR_NA 0x7fff

void aquacomputer_task(void *pvParameters);

void aquacomputer_set_temp_c(unsigned ch, float c);
void aquacomputer_set_fan_rpm(unsigned ch, uint16_t rpm);
void aquacomputer_set_flow_dl_h(uint16_t flow);

// Last PWM written by the host, 0-255. 0 if never set.
uint8_t aquacomputer_pwm(unsigned ch);
