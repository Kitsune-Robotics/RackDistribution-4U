// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2026 Kitsune Robotics

#include <linux/hid.h>
#include <linux/hwmon.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 12, 0)
#include <linux/unaligned.h>
#else
#include <asm/unaligned.h>
#endif

#include "rackdist_hid.h"

struct rackdist_data {
	struct device *hwmon;
	u8 status[RACKDIST_STATUS_REPORT_SIZE];
};

static const char *const temp_labels[] = { "Air Temperature", "Coolant Temperature", "Exhaust Temperature" };
static const char *const fan_labels[] = {
	"Pump", "Radiator 1", "Radiator 2", "Radiator 3",
	"Radiator 4", "Fan 6", "Fan 7", "Fan 8",
};

static umode_t rackdist_is_visible(const void *data, enum hwmon_sensor_types type,
				   u32 attr, int channel)
{
	if (type == hwmon_temp && channel < RACKDIST_NUM_TEMPS &&
	    (attr == hwmon_temp_input || attr == hwmon_temp_label))
		return 0444;
	if (type == hwmon_fan && channel < RACKDIST_NUM_FANS &&
	    (attr == hwmon_fan_input || attr == hwmon_fan_label))
		return 0444;
	if (type == hwmon_pwm && channel < RACKDIST_NUM_FANS &&
	    attr == hwmon_pwm_input)
		return 0444;
	return 0;
}

static int rackdist_read_string(struct device *dev, enum hwmon_sensor_types type,
				u32 attr, int channel, const char **str)
{
	if (type == hwmon_temp && attr == hwmon_temp_label)
		*str = temp_labels[channel];
	else if (type == hwmon_fan && attr == hwmon_fan_label)
		*str = fan_labels[channel];
	else
		return -EOPNOTSUPP;
	return 0;
}

static int rackdist_read(struct device *dev, enum hwmon_sensor_types type,
			 u32 attr, int channel, long *val)
{
	struct rackdist_data *priv = dev_get_drvdata(dev);
	s16 temp;

	switch (type) {
	case hwmon_temp:
		temp = get_unaligned_le16(priv->status + RACKDIST_OFF_TEMP +
					  channel * 2);
		if (temp == (s16)RACKDIST_TEMP_NA)
			return -ENODATA;
		*val = (long)temp * 10;
		return 0;
	case hwmon_fan:
		*val = get_unaligned_le16(priv->status + RACKDIST_OFF_FAN_RPM +
					  channel * 2);
		return 0;
	case hwmon_pwm:
		*val = priv->status[RACKDIST_OFF_FAN_PWM + channel];
		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

static const struct hwmon_ops rackdist_hwmon_ops = {
	.is_visible = rackdist_is_visible,
	.read = rackdist_read,
	.read_string = rackdist_read_string,
};

static const struct hwmon_channel_info *const rackdist_info[] = {
	HWMON_CHANNEL_INFO(temp,
			   HWMON_T_INPUT | HWMON_T_LABEL,
			   HWMON_T_INPUT | HWMON_T_LABEL,
			   HWMON_T_INPUT | HWMON_T_LABEL),
	HWMON_CHANNEL_INFO(fan,
			   HWMON_F_INPUT | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_LABEL),
	HWMON_CHANNEL_INFO(pwm,
			   HWMON_PWM_INPUT, HWMON_PWM_INPUT, HWMON_PWM_INPUT,
			   HWMON_PWM_INPUT, HWMON_PWM_INPUT, HWMON_PWM_INPUT,
			   HWMON_PWM_INPUT, HWMON_PWM_INPUT),
	NULL
};

static const struct hwmon_chip_info rackdist_chip_info = {
	.ops = &rackdist_hwmon_ops,
	.info = rackdist_info,
};

static int rackdist_raw_event(struct hid_device *hdev, struct hid_report *report,
			      u8 *data, int size)
{
	struct rackdist_data *priv = hid_get_drvdata(hdev);

	if (report->id == RACKDIST_STATUS_REPORT_ID &&
	    size >= RACKDIST_STATUS_REPORT_SIZE)
		memcpy(priv->status, data, RACKDIST_STATUS_REPORT_SIZE);
	return 0;
}

static int rackdist_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	struct rackdist_data *priv;
	int ret;

	priv = devm_kzalloc(&hdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	hid_set_drvdata(hdev, priv);

	ret = hid_parse(hdev);
	if (ret)
		return ret;

	ret = hid_hw_start(hdev, HID_CONNECT_HIDRAW);
	if (ret)
		return ret;

	ret = hid_hw_open(hdev);
	if (ret)
		goto err_stop;

	priv->hwmon = hwmon_device_register_with_info(&hdev->dev, "rackdist",
						      priv, &rackdist_chip_info,
						      NULL);
	if (IS_ERR(priv->hwmon)) {
		ret = PTR_ERR(priv->hwmon);
		goto err_close;
	}

	return 0;

err_close:
	hid_hw_close(hdev);
err_stop:
	hid_hw_stop(hdev);
	return ret;
}

static void rackdist_remove(struct hid_device *hdev)
{
	struct rackdist_data *priv = hid_get_drvdata(hdev);

	hwmon_device_unregister(priv->hwmon);
	hid_hw_close(hdev);
	hid_hw_stop(hdev);
}

static const struct hid_device_id rackdist_table[] = {
	{ HID_USB_DEVICE(RACKDIST_USB_VID, RACKDIST_USB_PID) },
	{ }
};
MODULE_DEVICE_TABLE(hid, rackdist_table);

static struct hid_driver rackdist_driver = {
	.name = "rackdist",
	.id_table = rackdist_table,
	.probe = rackdist_probe,
	.remove = rackdist_remove,
	.raw_event = rackdist_raw_event,
};
module_hid_driver(rackdist_driver);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Kitsune Robotics");
MODULE_DESCRIPTION("RackDistribution-4U hwmon");
MODULE_VERSION("0.1.0");
