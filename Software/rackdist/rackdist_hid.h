/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2026 Kitsune Robotics */
#ifndef RACKDIST_HID_H
#define RACKDIST_HID_H

#define RACKDIST_USB_VID 0x1209
#define RACKDIST_USB_PID 0x0001

#define RACKDIST_NUM_TEMPS 3
#define RACKDIST_NUM_FANS 8
#define RACKDIST_TEMP_NA 0x7fff /* s16 centidegC */

#define RACKDIST_STATUS_REPORT_ID 0x01
#define RACKDIST_STATUS_REPORT_SIZE 44
#define RACKDIST_OFF_TEMP 12
#define RACKDIST_OFF_FAN_RPM 18
#define RACKDIST_OFF_FAN_PWM 34

#endif
