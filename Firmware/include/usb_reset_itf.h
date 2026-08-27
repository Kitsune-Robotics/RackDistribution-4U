#pragma once

#include "parameters.h"

#if ENABLE_UF2_LOADER
#define PICO_USB_RESET_MS_OS_20_DESCRIPTOR_ITF 4
#else
#define PICO_USB_RESET_MS_OS_20_DESCRIPTOR_ITF 3
#endif
