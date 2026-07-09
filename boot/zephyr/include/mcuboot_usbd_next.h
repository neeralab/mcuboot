/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal USB device stack next (USBD) glue for MCUboot serial recovery.
 */

#ifndef MCUBOOT_USBD_NEXT_H_
#define MCUBOOT_USBD_NEXT_H_

/**
 * Set up descriptors, initialize and enable USB device support.
 *
 * @return 0 on success, negative errno on failure.
 */
int mcuboot_usbd_next_start(void);

/**
 * Disable and shut down USB before chain-loading the application.
 */
void mcuboot_usbd_next_stop(void);

#endif /* MCUBOOT_USBD_NEXT_H_ */
