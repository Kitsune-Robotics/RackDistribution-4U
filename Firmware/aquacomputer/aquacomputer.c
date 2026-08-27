#include "aquacomputer.h"

#include "analog.h"
#include "msc_uf2.h"
#include "tusb.h"

#include "FreeRTOS.h"
#include "task.h"

#include "pico/unique_id.h"

#include <math.h>
#include <string.h>

#define AQC_SERIAL_OFFSET 0x03
#define AQC_FIRMWARE_OFFSET 0x0d
#define AQC_POWER_CYCLES_OFFSET 0x18
#define AQC_TEMP_OFFSET 0x34
#define AQC_VIRTUAL_TEMP_OFFSET 0x3c
#define AQC_NUM_VIRTUAL_TEMPS 16
#define AQC_FLOW_OFFSET 0x6e

static const uint16_t k_fan_offsets[AQC_NUM_FANS] = {0x70, 0x7d, 0x8a, 0x97};
static const uint16_t k_pwm_offsets[AQC_NUM_FANS] = {0x37, 0x8c, 0xe1, 0x136};

static uint16_t g_fan_rpm[AQC_NUM_FANS];
static uint16_t g_temp_raw[AQC_NUM_TEMPS] = {0, AQC_SENSOR_NA, AQC_SENSOR_NA,
                                             AQC_SENSOR_NA};
static uint16_t g_flow_dl_h;
static uint8_t g_ctrl[AQC_CTRL_REPORT_SIZE];
static uint8_t g_save[AQC_SAVE_REPORT_SIZE];
static uint32_t g_power_cycles = 1;

static void put_be16(uint8_t *p, uint16_t v) {
  p[0] = (uint8_t)(v >> 8);
  p[1] = (uint8_t)v;
}

static void put_be32(uint8_t *p, uint32_t v) {
  p[0] = (uint8_t)(v >> 24);
  p[1] = (uint8_t)(v >> 16);
  p[2] = (uint8_t)(v >> 8);
  p[3] = (uint8_t)v;
}

static uint16_t get_be16(const uint8_t *p) {
  return (uint16_t)((p[0] << 8) | p[1]);
}

static uint16_t temp_to_aqc(float c) {
  if (!isfinite(c) || c < -50.0f || c > 150.0f) {
    return AQC_SENSOR_NA;
  }
  // Kernel does raw * 10 → millidegC, so this is 0.01 C
  int t = (int)(c * 100.0f);
  if (t < 0) {
    t = 0;
  }
  if (t > 0x7ffe) {
    t = 0x7ffe;
  }
  return (uint16_t)t;
}

void aquacomputer_set_temp_c(unsigned ch, float c) {
  if (ch == 0 || ch >= AQC_NUM_TEMPS) {
    return;
  }
  g_temp_raw[ch] = temp_to_aqc(c);
}

void aquacomputer_set_fan_rpm(unsigned ch, uint16_t rpm) {
  if (ch < AQC_NUM_FANS) {
    g_fan_rpm[ch] = rpm;
  }
}

void aquacomputer_set_flow_dl_h(uint16_t flow) { g_flow_dl_h = flow; }

uint8_t aquacomputer_pwm(unsigned ch) {
  if (ch >= AQC_NUM_FANS) {
    return 0;
  }
  uint16_t centi = get_be16(&g_ctrl[k_pwm_offsets[ch]]);
  return (uint8_t)((centi * 255u + 5000u) / 10000u);
}

static void fill_status(uint8_t *buf) {
  memset(buf, 0, AQC_STATUS_REPORT_SIZE);
  buf[0] = AQC_STATUS_REPORT_ID;

  pico_unique_board_id_t id;
  pico_get_unique_board_id(&id);
  put_be16(buf + AQC_SERIAL_OFFSET, (uint16_t)((id.id[0] << 8) | id.id[1]));
  put_be16(buf + AQC_SERIAL_OFFSET + 2, (uint16_t)((id.id[2] << 8) | id.id[3]));
  put_be16(buf + AQC_FIRMWARE_OFFSET, 1);
  put_be32(buf + AQC_POWER_CYCLES_OFFSET, g_power_cycles);

  put_be16(buf + AQC_TEMP_OFFSET, temp_to_aqc(analog_tsensor1_c()));
  for (unsigned i = 1; i < AQC_NUM_TEMPS; i++) {
    put_be16(buf + AQC_TEMP_OFFSET + i * 2, g_temp_raw[i]);
  }
  for (unsigned i = 0; i < AQC_NUM_VIRTUAL_TEMPS; i++) {
    put_be16(buf + AQC_VIRTUAL_TEMP_OFFSET + i * 2, AQC_SENSOR_NA);
  }

  put_be16(buf + AQC_FLOW_OFFSET, g_flow_dl_h);

  for (unsigned i = 0; i < AQC_NUM_FANS; i++) {
    uint8_t *fan = buf + k_fan_offsets[i];
    put_be16(fan + 0, 0);
    put_be16(fan + 2, 0);
    put_be16(fan + 4, 0);
    put_be16(fan + 6, 0);
    put_be16(fan + 8, g_fan_rpm[i]);
  }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
  (void)instance;
  (void)report_type;

  if (report_id == AQC_CTRL_REPORT_ID) {
    uint16_t n = (uint16_t)(AQC_CTRL_REPORT_SIZE - 1);
    if (n > reqlen) {
      n = reqlen;
    }
    memcpy(buffer, g_ctrl + 1, n);
    return n;
  }
  if (report_id == AQC_SAVE_REPORT_ID) {
    uint16_t n = (uint16_t)(AQC_SAVE_REPORT_SIZE - 1);
    if (n > reqlen) {
      n = reqlen;
    }
    memcpy(buffer, g_save + 1, n);
    return n;
  }
  if (report_id == AQC_STATUS_REPORT_ID) {
    uint8_t tmp[AQC_STATUS_REPORT_SIZE];
    fill_status(tmp);
    uint16_t n = (uint16_t)(AQC_STATUS_REPORT_SIZE - 1);
    if (n > reqlen) {
      n = reqlen;
    }
    memcpy(buffer, tmp + 1, n);
    return n;
  }
  return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
  (void)instance;
  (void)report_type;

  if (report_id == AQC_CTRL_REPORT_ID) {
    if (bufsize > AQC_CTRL_REPORT_SIZE - 1) {
      bufsize = AQC_CTRL_REPORT_SIZE - 1;
    }
    g_ctrl[0] = AQC_CTRL_REPORT_ID;
    memcpy(g_ctrl + 1, buffer, bufsize);
  } else if (report_id == AQC_SAVE_REPORT_ID) {
    if (bufsize > AQC_SAVE_REPORT_SIZE - 1) {
      bufsize = AQC_SAVE_REPORT_SIZE - 1;
    }
    g_save[0] = AQC_SAVE_REPORT_ID;
    memcpy(g_save + 1, buffer, bufsize);
  }
}

void aquacomputer_task(void *pvParameters) {
  (void)pvParameters;

  g_ctrl[0] = AQC_CTRL_REPORT_ID;
  g_save[0] = AQC_SAVE_REPORT_ID;

  while (true) {
    if (msc_uf2_ready_to_apply()) {
      vTaskDelay(pdMS_TO_TICKS(250));
      msc_uf2_apply();
    }
    uint8_t report[AQC_STATUS_REPORT_SIZE];
    fill_status(report);
    if (tud_hid_ready()) {
      tud_hid_report(AQC_STATUS_REPORT_ID, report + 1, AQC_STATUS_REPORT_SIZE - 1);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
