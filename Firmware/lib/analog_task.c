#include "analog.h"

#include "FreeRTOS.h"
#include "hardware/adc.h"
#include "parameters.h"
#include "pindefs.h"
#include "task.h"

#include <math.h>
#include <stdio.h>

static volatile float g_tsensor1_c;

float analog_tsensor1_c(void) { return g_tsensor1_c; }

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

void analog_task(void *pvParameters) {
  (void)pvParameters;

  adc_init();
  adc_gpio_init(TSENSOR1_PIN);

  while (true) {
    uint16_t raw = adc_read_avg(TSENSOR1_ADC_CH, 8);
    float r = ntc_ohms_from_adc(raw);
    g_tsensor1_c = ntc_c_from_ohms(r) + TSENSOR1_OFFSET_C;

    printf("t1=%.1f C  R=%.0f\n", (double)g_tsensor1_c, (double)r);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
