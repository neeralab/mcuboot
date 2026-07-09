/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * USB device stack next (USBD) setup for MCUboot CDC ACM serial recovery.
 */

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/usb/usbd.h>

#include "mcuboot_usbd_next.h"

#ifndef CONFIG_USB_DEVICE_VID
#define CONFIG_USB_DEVICE_VID 0x2fe3
#endif
#ifndef CONFIG_USB_DEVICE_PID
#define CONFIG_USB_DEVICE_PID 0x0001
#endif

#define MCUBOOT_USB_VID CONFIG_USB_DEVICE_VID
#define MCUBOOT_USB_PID CONFIG_USB_DEVICE_PID

#ifndef CONFIG_USB_DEVICE_MANUFACTURER
#define CONFIG_USB_DEVICE_MANUFACTURER "Zephyr"
#endif
#ifndef CONFIG_USB_DEVICE_PRODUCT
#define CONFIG_USB_DEVICE_PRODUCT "MCUboot CDC ACM"
#endif

#define MCUBOOT_USB_MANUFACTURER CONFIG_USB_DEVICE_MANUFACTURER
#define MCUBOOT_USB_PRODUCT      CONFIG_USB_DEVICE_PRODUCT

#define MCUBOOT_USB_MAX_POWER    250
#define MCUBOOT_USB_ATTRIBUTES   0U

static const char *const blocklist[] = {
	"dfu_dfu",
	NULL,
};

USBD_DEVICE_DEFINE(mcuboot_usbd, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)),
		   MCUBOOT_USB_VID, MCUBOOT_USB_PID);

USBD_DESC_LANG_DEFINE(mcuboot_lang);
USBD_DESC_MANUFACTURER_DEFINE(mcuboot_mfr, MCUBOOT_USB_MANUFACTURER);
USBD_DESC_PRODUCT_DEFINE(mcuboot_product, MCUBOOT_USB_PRODUCT);

USBD_DESC_CONFIG_DEFINE(fs_cfg_desc, "FS Configuration");
USBD_DESC_CONFIG_DEFINE(hs_cfg_desc, "HS Configuration");

USBD_CONFIGURATION_DEFINE(mcuboot_fs_config, MCUBOOT_USB_ATTRIBUTES, MCUBOOT_USB_MAX_POWER,
			  &fs_cfg_desc);
USBD_CONFIGURATION_DEFINE(mcuboot_hs_config, MCUBOOT_USB_ATTRIBUTES, MCUBOOT_USB_MAX_POWER,
			  &hs_cfg_desc);

static void mcuboot_fix_code_triple(struct usbd_context *uds_ctx, const enum usbd_speed speed)
{
	usbd_device_set_code_triple(uds_ctx, speed, USB_BCC_MISCELLANEOUS, 0x02, 0x01);
}

static int mcuboot_usbd_setup(void)
{
	int err;

	err = usbd_add_descriptor(&mcuboot_usbd, &mcuboot_lang);
	if (err != 0) {
		return err;
	}

	err = usbd_add_descriptor(&mcuboot_usbd, &mcuboot_mfr);
	if (err != 0) {
		return err;
	}

	err = usbd_add_descriptor(&mcuboot_usbd, &mcuboot_product);
	if (err != 0) {
		return err;
	}

	if (IS_ENABLED(CONFIG_USBD_MAX_SPEED_HIGH) &&
	    usbd_caps_speed(&mcuboot_usbd) == USBD_SPEED_HS) {
		err = usbd_add_configuration(&mcuboot_usbd, USBD_SPEED_HS, &mcuboot_hs_config);
		if (err != 0) {
			return err;
		}

		err = usbd_register_all_classes(&mcuboot_usbd, USBD_SPEED_HS, 1, blocklist);
		if (err != 0) {
			return err;
		}

		mcuboot_fix_code_triple(&mcuboot_usbd, USBD_SPEED_HS);
	}

	err = usbd_add_configuration(&mcuboot_usbd, USBD_SPEED_FS, &mcuboot_fs_config);
	if (err != 0) {
		return err;
	}

	err = usbd_register_all_classes(&mcuboot_usbd, USBD_SPEED_FS, 1, blocklist);
	if (err != 0) {
		return err;
	}

	mcuboot_fix_code_triple(&mcuboot_usbd, USBD_SPEED_FS);

	return 0;
}

int mcuboot_usbd_next_start(void)
{
	int err;

	err = mcuboot_usbd_setup();
	if (err != 0) {
		return err;
	}

	err = usbd_init(&mcuboot_usbd);
	if (err != 0) {
		return err;
	}

	if (!usbd_can_detect_vbus(&mcuboot_usbd)) {
		err = usbd_enable(&mcuboot_usbd);
	}

	return err;
}

void mcuboot_usbd_next_stop(void)
{
	(void)usbd_disable(&mcuboot_usbd);
	(void)usbd_shutdown(&mcuboot_usbd);
}
