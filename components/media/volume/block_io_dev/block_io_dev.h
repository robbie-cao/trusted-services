/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MEDIA_BLOCK_IO_DEV_H
#define MEDIA_BLOCK_IO_DEV_H

#include <common/uuid/uuid.h>
#include <service/block_storage/block_store/block_store.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Export tf-a version with C++ linkage support.
 */
#include <drivers/io/io_driver.h>

/**
 * Provides a tf-a compatible io device that presents a block storage partition
 * as a single volume. Access to the underlying storage is handled by an associated
 * block_store. The block_store could be any concrete block_store.
 */
struct block_io_dev
{
	io_dev_info_t dev_info;
	size_t file_pos;
	size_t size;
	struct block_store *block_store;
	struct uuid_octets partition_guid;
	storage_partition_handle_t partition_handle;
	struct storage_partition_info partition_info;
};

/**
 * @brief  Initialize an block_io_dev instance
 *
 * @param[in] this_instance    The subject block_io_dev
 * @param[in] block_store      The associated block_store
 * @param[in] partition_guid   The partition GUID
 * @param[out] dev_handle	   Device handle used by tf-a components
 * @param[out] spec			   Spec passed on io_open
 *
 * @return 0 on success
 */
int block_io_dev_init(
	struct block_io_dev *this_instance,
	struct block_store *block_store,
	const struct uuid_octets *partition_guid,
	uintptr_t *dev_handle,
	uintptr_t *spec);

/**
 * @brief  De-initialize an block_io_dev instance
 *
 * @param[in] this_instance    The subject block_io_dev
 */
void block_io_dev_deinit(
	struct block_io_dev *this_instance);

/**
 * @brief  Set the partition GUID
 *
 * Modifies the partition GUID. This will be used to identify the target
 * storage partition on a subsequent call to io_dev_open.
 *
 * @param[in] this_instance    The subject block_io_dev
 * @param[in] partition_guid   The partition GUID
 */
void block_io_dev_set_partition_guid(
	struct block_io_dev *this_instance,
	const struct uuid_octets *partition_guid);

#ifdef __cplusplus
}
#endif

#endif /* MEDIA_BLOCK_IO_DEV_H */
