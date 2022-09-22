/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <plat/common/platform.h>
#include "volume_index.h"

#ifndef VOLUME_INDEX_MAX_ENTRIES
#define VOLUME_INDEX_MAX_ENTRIES		(4)
#endif

/**
 * Singleton index of volume IDs to IO devices.
 */
static struct
{
	size_t size;
	struct
	{
		unsigned int volume_id;
		uintptr_t dev_handle;
		uintptr_t volume_spec;
	} entries[VOLUME_INDEX_MAX_ENTRIES];

} volume_index;

/**
 * @brief  Gets a device for volume IO operations
 *
 * @param[in]  volume_id 	Identifies the image
 * @param[out] dev_handle 	Handle for IO operations
 * @param[out] volume_spec	Opaque configuration data
 *
 * This function realizes the interface expected by tf-a components to
 * provide a concrete IO device for the specified volume ID. When used in
 * TS deployments, the set of IO devices required for a deployment
 * are registered during service configuration.
 */
int plat_get_image_source(
	unsigned int volume_id,
	uintptr_t *dev_handle,
	uintptr_t *volume_spec)
{
	int result = -1;

	for (size_t i = 0; i < volume_index.size; i++) {

		if (volume_index.entries[i].volume_id == volume_id) {

			*dev_handle = volume_index.entries[i].dev_handle;
			*volume_spec = volume_index.entries[i].volume_spec;

			result = 0;
			break;
		}
	}

	return result;
}

void volume_index_init(void)
{
	volume_index_clear();
}

void volume_index_clear(void)
{
	memset(&volume_index, 0, sizeof(volume_index));
}

int volume_index_add(
	unsigned int volume_id,
	uintptr_t dev_handle,
	uintptr_t volume_spec)
{
	int result = -1;

	if (volume_index.size < VOLUME_INDEX_MAX_ENTRIES)
	{
		size_t i = volume_index.size;

		++volume_index.size;
		volume_index.entries[i].volume_id = volume_id;
		volume_index.entries[i].dev_handle = dev_handle;
		volume_index.entries[i].volume_spec = volume_spec;

		result = 0;
	}

	return result;
}
