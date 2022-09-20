/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <plat/common/platform.h>

int plat_get_image_source(
	unsigned int image_id,
	uintptr_t *dev_handle,
	uintptr_t *image_spec)
{
	/* Needs to allow a TS factory method to define the image source */
	(void)image_id;
	*dev_handle = (uintptr_t)NULL;
	*image_spec = (uintptr_t)NULL;
	return -1;
}