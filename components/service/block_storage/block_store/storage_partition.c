/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <string.h>
#include "storage_partition.h"


void storage_partition_init(
	struct storage_partition *partition,
	const struct uuid_octets *partition_guid,
	size_t num_blocks,
	size_t block_size)
{
	memset(partition, 0, sizeof(struct storage_partition));

	partition->partition_guid = *partition_guid;
	partition->block_size = block_size;
	partition->num_blocks = num_blocks;
}

void storage_partition_deinit(
	struct storage_partition *partition)
{
	memset(partition, 0, sizeof(struct storage_partition));
}

void storage_partition_extend_whitelist(
	struct storage_partition *partition,
	uint32_t client_id)
{
	if (partition->whitelist_len < STORAGE_PARTITION_WHITELIST_LEN) {

		partition->whitelist[partition->whitelist_len] = client_id;
		++partition->whitelist_len;
	}
}

bool storage_partition_is_guid_matched(
	const struct storage_partition *partition,
	const struct uuid_octets *partition_guid)
{
	return (memcmp(&partition->partition_guid, partition_guid, sizeof(struct uuid_octets)) == 0);
}

bool storage_partition_is_access_permitted(
	const struct storage_partition *partition,
	uint32_t client_id)
{
	bool is_permitted = (partition->whitelist_len == 0);

	for (size_t i = 0; !is_permitted && i < partition->whitelist_len; ++i)
		is_permitted = (client_id == partition->whitelist[i]);

	return is_permitted;
}

bool storage_partition_is_lba_legal(
	const struct storage_partition *partition,
	uint32_t lba)
{
	return lba < partition->num_blocks;
}

size_t storage_partition_clip_length(
	const struct storage_partition *partition,
	uint32_t lba,
	size_t offset,
	size_t req_len)
{
	size_t clipped_len = 0;

	if (lba < partition->num_blocks) {

		size_t remaining_len = (partition->num_blocks - lba) * partition->block_size;

		remaining_len = (offset < remaining_len) ? remaining_len - offset : 0;
		clipped_len = (req_len > remaining_len) ? remaining_len : req_len;
	}

	return clipped_len;
}

size_t storage_partition_clip_num_blocks(
	const struct storage_partition *partition,
	uint32_t lba,
	size_t num_blocks)
{
	size_t clipped_num_blocks = 0;

	if (lba < partition->num_blocks) {

		size_t remaining_blocks = partition->num_blocks - lba;

		clipped_num_blocks = (num_blocks > remaining_blocks) ? remaining_blocks : num_blocks;
	}

	return clipped_num_blocks;
}
