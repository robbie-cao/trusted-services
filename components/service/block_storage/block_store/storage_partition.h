/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef STORAGE_PARTITION_H
#define STORAGE_PARTITION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "common/uuid/uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Default maximum configurable whitelist length */
#ifndef STORAGE_PARTITION_WHITELIST_LEN
#define STORAGE_PARTITION_WHITELIST_LEN    (4)
#endif

/**
 * \brief Common storage partition structure
 *
 * A block store may present one or more storage partitions. This structure may
 * be used by concrete block_store implementations to describe a storage partition
 * in a generic way.
 */
struct storage_partition
{
	/* Unique partition GUID */
	struct uuid_octets partition_guid;

	/* Block size in bytes */
	size_t block_size;

	/* The number of contiguous blocks from LBA zero */
	size_t num_blocks;

	/* Backend storage block that corresponds to LBA zero */
	uint32_t base_lba;

	/* Whitelist of client IDs corresponding to clients that are permitted
	 * to access the partition. A zero length list is treated as a wildcard
	 * where any client is permitted access. */
	size_t whitelist_len;
	uint32_t whitelist[STORAGE_PARTITION_WHITELIST_LEN];
};

/**
 * \brief Default storage_partition initialization function
 *
 * Initializes a storage_partition with an empty whitelist and a one-to-one
 * LBA mapping to backend storage.
 *
 * \param[in]  partition       The subject storage_partition
 * \param[in]  partition_guid  The unique partition GUID
 * \param[in]  num_blocks      The number of contiguous blocks
 * \param[in]  block_size      Block size in bytes
 */
void storage_partition_init(
	struct storage_partition *partition,
	const struct uuid_octets *partition_guid,
	size_t num_blocks,
	size_t block_size);

/**
 * \brief Cleans up a previously initialized storage_partition
 *
 * Should be called when the storage_partition is no longer needed.
 *
 * \param[in]  partition       The subject storage_partition
 */
void storage_partition_deinit(
	struct storage_partition *partition);

/**
 * \brief Extend the whitelist
 *
 * \param[in]  partition    The subject storage_partition
 * \param[in]  client_id    The client ID to add
 */
void storage_partition_extend_whitelist(
	struct storage_partition *partition,
	uint32_t client_id);

/**
 * \brief Check if unique partition GUID matches
 *
 * \param[in]  partition       The subject storage_partition
 * \param[in]  partition_guid  The unique partition GUID
 * \return     True if GUID matches the storage partition GUID
 */
bool storage_partition_is_guid_matched(
	const struct storage_partition *partition,
	const struct uuid_octets *partition_guid);

/**
 * \brief Check if access to the storage partition is permitted
 *
 * \param[in]  partition    The subject storage_partition
 * \param[in]  client_id    The requesting client ID
 * \return     True if access permitted
 */
bool storage_partition_is_access_permitted(
	const struct storage_partition *partition,
	uint32_t client_id);

/**
 * \brief Check if lba is legal for partition
 *
 * \param[in]  partition    The subject storage_partition
 * \param[in]  lba          The LBA to check
 * \return     True if legal
 */
bool storage_partition_is_lba_legal(
	const struct storage_partition *partition,
	uint32_t lba);

/**
 * \brief Clip the length if it exceeds the limits of the partition
 *
 * \param[in]  partition    The subject storage_partition
 * \param[in]  lba          The start LBA
 * \param[in]  offset       Byte off set from start of block
 * \param[in]  req_len      Requested length
 * \return     Clipped length if req_len exceeds limit of partition
 */
size_t storage_partition_clip_length(
	const struct storage_partition *partition,
	uint32_t lba,
	size_t offset,
	size_t req_len);

/**
 * \brief Clip the number of blocks if it exceeds the limits of the partition
 *
 * \param[in]  partition    The subject storage_partition
 * \param[in]  lba          The start LBA
 * \param[in]  num_blocks   Requested num_blocks
 * \return     Clipped num_blocks if request number exceeds the limits of the partition
 */
size_t storage_partition_clip_num_blocks(
	const struct storage_partition *partition,
	uint32_t lba,
	size_t num_blocks);

#ifdef __cplusplus
}
#endif

#endif /* STORAGE_PARTITION_H */
