#include "hid.h"

#include "analog.h"
#include "fans.h"
#include "parameters.h"
#include "rackdist_hid.h"
#if ENABLE_UF2_LOADER
#include "msc_uf2.h"
#endif
#include "tusb.h"

#include "FreeRTOS.h"
#include "task.h"

#include "pico/time.h"

#include <math.h>
#include <string.h>

static volatile uint32_t g_hid_consumed_us;

#define HID_CONSUMED_US 3000000u

static void mark_hid_consumed(void) { g_hid_consumed_us = time_us_32(); }

bool hid_consumed(void) {
  uint32_t then = g_hid_consumed_us;
  return then != 0 && (time_us_32() - then) < HID_CONSUMED_US;
}

static void put_le16(uint8_t *p, uint16_t v) {
  p[0] = (uint8_t)v;
  p[1] = (uint8_t)(v >> 8);
}

static uint16_t temp_to_raw(float c) {
  if (!isfinite(c) || c < -50.0f || c > 150.0f) {
    return RACKDIST_TEMP_NA;
  }
  int t = (int)(c * 100.0f);
  if (t < -0x7ffe) {
    t = -0x7ffe;
  }
  if (t > 0x7ffe) {
    t = 0x7ffe;
  }
  return (uint16_t)t;
}

static void fill_status(uint8_t *buf) {
  memset(buf, 0, RACKDIST_STATUS_REPORT_SIZE);
  buf[0] = RACKDIST_STATUS_REPORT_ID;

  put_le16(buf + RACKDIST_OFF_TEMP + 0, temp_to_raw(analog_air_c()));
  put_le16(buf + RACKDIST_OFF_TEMP + 2, temp_to_raw(analog_coolant_c()));
  put_le16(buf + RACKDIST_OFF_TEMP + 4, temp_to_raw(analog_exhaust_c()));

  for (unsigned i = 0; i < RACKDIST_NUM_FANS; i++) {
    put_le16(buf + RACKDIST_OFF_FAN_RPM + i * 2, fans_rpm(i));
    buf[RACKDIST_OFF_FAN_PWM + i] = fans_pwm_get(i);
  }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
  (void)instance;
  (void)report_type;

  if (report_id != RACKDIST_STATUS_REPORT_ID) {
    return 0;
  }
  uint8_t tmp[RACKDIST_STATUS_REPORT_SIZE];
  fill_status(tmp);
  uint16_t n = (uint16_t)(RACKDIST_STATUS_REPORT_SIZE - 1);
  if (n > reqlen) {
    n = reqlen;
  }
  memcpy(buffer, tmp + 1, n);
  mark_hid_consumed();
  return n;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)bufsize;
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report,
                                uint16_t len) {
  (void)instance;
  (void)len;
  if (report && report[0] == RACKDIST_STATUS_REPORT_ID) {
    mark_hid_consumed();
  }
}

void hid_task(void *pvParameters) {
  (void)pvParameters;

  while (true) {
#if ENABLE_UF2_LOADER
    if (msc_uf2_ready_to_apply()) {
      vTaskDelay(pdMS_TO_TICKS(250));
      msc_uf2_apply();
    }
#endif
    uint8_t report[RACKDIST_STATUS_REPORT_SIZE];
    fill_status(report);
    if (tud_hid_ready()) {
      tud_hid_report(RACKDIST_STATUS_REPORT_ID, report + 1,
                     RACKDIST_STATUS_REPORT_SIZE - 1);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
