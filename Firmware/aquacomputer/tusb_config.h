#pragma once

#include "parameters.h"

#define CFG_TUSB_RHPORT0_MODE OPT_MODE_DEVICE

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE 64
#endif

#define CFG_TUD_CDC 1
#define CFG_TUD_HID 1
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 0

#define CFG_TUD_CDC_RX_BUFSIZE 64
#define CFG_TUD_CDC_TX_BUFSIZE 64
#define CFG_TUD_CDC_EP_BUFSIZE 64

#if ENABLE_UF2_LOADER
#define CFG_TUD_MSC 1
#define CFG_TUD_MSC_EP_BUFSIZE 512
#else
#define CFG_TUD_MSC 0
#endif

// Quadro feature report 0x03 is 0x3c1 bytes including report ID.
#define CFG_TUD_HID_EP_BUFSIZE 1024
