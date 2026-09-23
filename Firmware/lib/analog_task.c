#include "analog.h"

#include "FreeRTOS.h"
#include "hardware/adc.h"
#include "parameters.h"
#include "pindefs.h"
#include "task.h"

#include <math.h>
#include <stdio.h>

static volatile float g_coolant_c = NAN;
static volatile float g_air_c = NAN;
static volatile float g_exhaust_c = NAN;

float analog_coolant_c(void) { return g_coolant_c; } // Coolant Temperature

float analog_exhaust_c(void) { return g_exhaust_c; } // Exhaust Temperature

float analog_air_c(void) {
  // Ambient Temperature (or fallback if theres an issue)
  float t = g_air_c;
  return isfinite(t) ? t : AMBIENT_FALLBACK_C;
}

float analog_control_c(void) {
  // What we'll use as the control temp
  // TODO: maybe a fusion at some point?
  float t = analog_coolant_c();
  return isfinite(t) ? t : analog_air_c();
}

static uint16_t adc_read_avg(uint ch, unsigned n) {
  adc_select_input(ch);
  uint32_t sum = 0;
  for (unsigned i = 0; i < n; i++) {
    sum += adc_read();
  }
  return (uint16_t)(sum / n);
}

static float ntc_ohms_from_adc(uint16_t raw) {
  // 3V3 -- 10k -- adc -- ntc -- gnd
  if (raw == 0) {
    return 0.0f;
  }
  if (raw >= 4095) {
    return 1.0e6f;
  }
  float v = (float)raw / 4095.0f;
  return TSENSOR_PULLUP_OHMS * v / (1.0f - v);
}

static float ntc_c_from_ohms(float r) {
  if (r < 1.0f) {
    r = 1.0f;
  }
  const float inv_t =
      (1.0f / 298.15f) + (1.0f / NTC_BETA) * logf(r / NTC_R25_OHMS);
  return (1.0f / inv_t) - 273.15f;
}

static float read_ntc_c(uint adc_ch, float offset) {
  uint16_t raw = adc_read_avg(adc_ch, 12); // TODO: make n tunable
  if (raw <= NTC_SHORT_RAW || raw >= NTC_OPEN_RAW) {
    return NAN;
  }
  return ntc_c_from_ohms(ntc_ohms_from_adc(raw)) + offset;
}

void analog_task(void *pvParameters) {
  (void)pvParameters;

  adc_init();
  adc_gpio_init(TSENSOR_0_PIN);
  adc_gpio_init(TSENSOR_1_PIN);
  adc_gpio_init(TSENSOR_2_PIN);

  while (true) {
    g_coolant_c = read_ntc_c(TSENSOR_0_ADC_CH, TSENSOR0_OFFSET_C);
    g_air_c = read_ntc_c(TSENSOR_1_ADC_CH, TSENSOR1_OFFSET_C);
    g_exhaust_c = read_ntc_c(TSENSOR_2_ADC_CH, TSENSOR2_OFFSET_C);

    printf("cool=%.1f air=%.1f exh=%.1f\n", (double)g_coolant_c,
           (double)g_air_c, (double)g_exhaust_c);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
