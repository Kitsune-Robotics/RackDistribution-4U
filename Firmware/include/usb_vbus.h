#pragma once

#include "hardware/gpio.h"
#include "pindefs.h"

#include <stdbool.h>

static inline void usb_vbus_init(void) {
  gpio_init(USB_VBUS_PIN);
  gpio_set_dir(USB_VBUS_PIN, GPIO_IN);
  gpio_disable_pulls(USB_VBUS_PIN);
}

static inline bool usb_vbus_present(void) { return gpio_get(USB_VBUS_PIN); }