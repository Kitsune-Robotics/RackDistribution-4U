#include "aquacomputer.h"

#include "pico/unique_id.h"
#include "pico/usb_reset.h"
#include "tusb.h"

#define USBD_VID AQC_USB_VID
#define USBD_PID AQC_USB_PID

#define USBD_ITF_CDC 0
#define USBD_ITF_HID 2
#define USBD_ITF_MSC 3
#define USBD_ITF_RPI_RESET 4
#define USBD_ITF_MAX 5

#if PICO_USB_RESET_SUPPORT_MS_OS_20_DESCRIPTOR
static_assert(USBD_ITF_RPI_RESET == PICO_USB_RESET_MS_OS_20_DESCRIPTOR_ITF,
              "reset interface # must match PICO_USB_RESET_MS_OS_20_DESCRIPTOR_ITF");
#endif

#define USBD_CDC_EP_CMD 0x81
#define USBD_CDC_EP_OUT 0x02
#define USBD_CDC_EP_IN 0x82
#define USBD_HID_EP_IN 0x83
#define USBD_MSC_EP_OUT 0x04
#define USBD_MSC_EP_IN 0x84

#define USBD_CDC_CMD_MAX_SIZE 8
#define USBD_CDC_IN_OUT_MAX_SIZE 64
#define USBD_HID_EP_SIZE 64

#define USBD_STR_0 0
#define USBD_STR_MANUF 1
#define USBD_STR_PRODUCT 2
#define USBD_STR_SERIAL 3
#define USBD_STR_CDC 4
#define USBD_STR_HID 5
#define USBD_STR_MSC 6
#define USBD_STR_RPI_RESET 7

#define USBD_DESC_LEN                                                                  \
  (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_DESC_LEN + TUD_MSC_DESC_LEN +      \
   TUD_RPI_RESET_DESC_LEN)

static const tusb_desc_device_t usbd_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
#if PICO_ENABLE_USB_RESET_VIA_VENDOR_INTERFACE &&                                      \
    PICO_USB_RESET_SUPPORT_MS_OS_20_DESCRIPTOR
    .bcdUSB = 0x0210,
#else
    .bcdUSB = 0x0200,
#endif
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUF,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

uint8_t const desc_hid_report[] = {
    HID_USAGE_PAGE_N(0xff00, 2),
    HID_USAGE(0x01),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
    HID_LOGICAL_MIN(0),
    HID_LOGICAL_MAX_N(255, 2),
    HID_REPORT_SIZE(8),

    HID_REPORT_ID(AQC_STATUS_REPORT_ID) HID_USAGE(0x01),
    HID_REPORT_COUNT(AQC_STATUS_REPORT_SIZE - 1),
    HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

    HID_REPORT_ID(AQC_SAVE_REPORT_ID) HID_USAGE(0x02),
    HID_REPORT_COUNT(AQC_SAVE_REPORT_SIZE - 1),
    HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

    HID_REPORT_ID(AQC_CTRL_REPORT_ID) HID_USAGE(0x03),
    HID_REPORT_COUNT_N(AQC_CTRL_REPORT_SIZE - 1, 2),
    HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,
};

static const uint8_t usbd_desc_cfg[USBD_DESC_LEN] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_0, USBD_DESC_LEN, 0, 250),
    TUD_CDC_DESCRIPTOR(USBD_ITF_CDC, USBD_STR_CDC, USBD_CDC_EP_CMD,
                       USBD_CDC_CMD_MAX_SIZE, USBD_CDC_EP_OUT, USBD_CDC_EP_IN,
                       USBD_CDC_IN_OUT_MAX_SIZE),
    TUD_HID_DESCRIPTOR(USBD_ITF_HID, USBD_STR_HID, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report), USBD_HID_EP_IN, USBD_HID_EP_SIZE, 10),
    TUD_MSC_DESCRIPTOR(USBD_ITF_MSC, USBD_STR_MSC, USBD_MSC_EP_OUT, USBD_MSC_EP_IN, 64),
    TUD_RPI_RESET_DESCRIPTOR(USBD_ITF_RPI_RESET, USBD_STR_RPI_RESET),
};

static char usbd_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

static const char *const usbd_desc_str[] = {
    [USBD_STR_MANUF] = "Aquacomputer",
    [USBD_STR_PRODUCT] = "quadro",
    [USBD_STR_SERIAL] = usbd_serial_str,
    [USBD_STR_CDC] = "Board CDC",
    [USBD_STR_HID] = "quadro",
    [USBD_STR_MSC] = "UF2",
    [USBD_STR_RPI_RESET] = "Reset",
};

const uint8_t *tud_descriptor_device_cb(void) {
  return (const uint8_t *)&usbd_desc_device;
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
  (void)index;
  return usbd_desc_cfg;
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
  (void)instance;
  return desc_hid_report;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;
#define USBD_DESC_STR_MAX 32
  static uint16_t desc_str[USBD_DESC_STR_MAX];

  if (!usbd_serial_str[0]) {
    pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));
  }

  uint8_t len;
  if (index == 0) {
    desc_str[1] = 0x0409;
    len = 1;
  } else {
    if (index >= sizeof(usbd_desc_str) / sizeof(usbd_desc_str[0])) {
      return NULL;
    }
    const char *str = usbd_desc_str[index];
    if (!str) {
      return NULL;
    }
    for (len = 0; len < USBD_DESC_STR_MAX - 1 && str[len]; ++len) {
      desc_str[1 + len] = str[len];
    }
  }

  desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * len + 2));
  return desc_str;
}
