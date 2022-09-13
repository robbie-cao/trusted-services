/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "block_store_factory.h"
#include "service/block_storage/block_store/device/semihosting/semihosting_block_store.h"
#include "service/block_storage/block_store/partitioned/partitioned_block_store.h"

struct block_store_assembly
{
	struct semihosting_block_store semihosting_block_store;
	struct partitioned_block_store partitioned_block_store;
};

struct block_store *semihosting_block_store_factory_create(void)
{
	struct block_store *product = NULL;
	struct block_store_assembly *assembly =
		(struct block_store_assembly*)malloc(sizeof(struct block_store_assembly));

	if (assembly) {

		struct uuid_octets back_store_guid;
		memset(&back_store_guid, 0, sizeof(back_store_guid));

		/* Initialise a semihosting_block_store to provide underlying storage */
		struct block_store *back_store = semihosting_block_store_init(
			&assembly->semihosting_block_store,
			"secure-flash.img");

		/* Stack a partitioned_block_store over the back store */
		product = partitioned_block_store_init(
			&assembly->partitioned_block_store,
			0,
			&back_store_guid,
			back_store,
			NULL);

		if (!product) {

			/* Something went wrong! */
			free(assembly);
		}
	}

	return product;
}

void semihosting_block_store_factory_destroy(struct block_store *block_store)
{
	if (block_store) {

		size_t offset_into_assembly =
			offsetof(struct block_store_assembly, partitioned_block_store) +
			offsetof(struct partitioned_block_store, base_block_store);

		struct block_store_assembly *assembly = (struct block_store_assembly*)
			((uint8_t*)block_store - offset_into_assembly);

		partitioned_block_store_deinit(&assembly->partitioned_block_store);
		semihosting_block_store_deinit(&assembly->semihosting_block_store);

		free(assembly);
	}
}
