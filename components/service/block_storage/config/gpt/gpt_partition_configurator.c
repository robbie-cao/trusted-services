/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include <common/uuid/uuid.h>
#include <media/disk/partition_table.h>
#include "gpt_partition_configurator.h"

static bool gpt_partition_config_listener(
	struct partitioned_block_store *subject,
	const struct uuid_octets *partition_guid,
	const struct storage_partition_info *back_store_info)
{
	/* Performs on-demand partition configuration on an attempt to
	 * open an unconfigured storage partition.
	 */
	bool is_configured = false;
	const partition_entry_t *partition_entry = NULL;

	/* Check if matching partition entry exists in loaded GPT */
	partition_entry = get_partition_entry_by_uuid((uuid_t *)partition_guid->octets);

	if (partition_entry &&
		back_store_info->block_size &&
		(partition_entry->length >= back_store_info->block_size) &&
		!(partition_entry->length % back_store_info->block_size)) {

		/* Partition entry exists and values look sane */
		uint32_t starting_lba =
			partition_entry->start / back_store_info->block_size;
		uint32_t ending_lba =
			starting_lba + (partition_entry->length / back_store_info->block_size) - 1;

		if (ending_lba >= starting_lba) {

			is_configured = partitioned_block_store_add_partition(
				subject, partition_guid,
				starting_lba, ending_lba,
				0, NULL);
		}
	}

	return is_configured;
}

bool gpt_partition_configure(
	struct partitioned_block_store *subject,
	unsigned int volume_id)
{
	int result = load_partition_table(volume_id);

	if (result == 0) {

		partitioned_block_store_attach_config_listener(
			subject,
			gpt_partition_config_listener);
	}

	return result == 0;
}
