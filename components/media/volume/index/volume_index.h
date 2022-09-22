/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VOLUME_INDEX_H
#define VOLUME_INDEX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize the volume index
 *
 * The volume_index is a singleton that holds the mapping of volume IDs
 * to concrete IO devices that can be used to access the volume. The
 * mappings are setup during deployment configuration to meet the IO needs
 * of the deployment. The volume_index realizes the tf-a function
 * plat_get_image_source() to make the mappings available to tf-a components.
 */
void volume_index_init(void);

/**
 * @brief  Clears the volume index
 *
 * Clears all mappings.
 */
void volume_index_clear(void);

/**
 * @brief  Add an entry to the volume index
 *
 * @param[in] volume_id   Volume identifier
 * @param[in] dev_handle  Device handle to use
 * @param[in] volume_spec  Additional information about the volume
 *
 * @return 0 if successful
 */
int volume_index_add(
	unsigned int volume_id,
	uintptr_t dev_handle,
	uintptr_t volume_spec);

#ifdef __cplusplus
}
#endif

#endif /* VOLUME_INDEX_H */
