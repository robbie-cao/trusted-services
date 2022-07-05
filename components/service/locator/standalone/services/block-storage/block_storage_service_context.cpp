/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cstring>
#include "service/block_storage/config/ref/ref_partition_configurator.h"
#include "service/block_storage/provider/serializer/packed-c/packedc_block_storage_serializer.h"
#include "block_storage_service_context.h"

block_storage_service_context::block_storage_service_context(const char *sn) :
	standalone_service_context(sn),
	m_block_storage_provider(),
	m_ram_block_store(),
	m_partitioned_block_store()
{

}

block_storage_service_context::~block_storage_service_context()
{

}

void block_storage_service_context::do_init()
{
	/* Initialize a ram_block_store to use as the back store */
	struct uuid_octets back_store_guid;
	memset(&back_store_guid, 0, sizeof(back_store_guid));

	struct block_store *back_store = ram_block_store_init(
		&m_ram_block_store,
		&back_store_guid,
		REF_PARTITION_BACK_STORE_SIZE,
		REF_PARTITION_BLOCK_SIZE);
	assert(back_store);

	/* Stack a partitioned_block_store over the back store */
	struct block_store *front_store = partitioned_block_store_init(
		&m_partitioned_block_store,
		0,
		&back_store_guid,
		back_store);
	assert(front_store);

	/* Use the reference partition configuration */
	ref_partition_configure(&m_partitioned_block_store);

	/* Initialise the block storage service provider */
	struct rpc_interface *rpc_iface = block_storage_provider_init(
		&m_block_storage_provider,
		front_store);
	assert(rpc_iface);

	block_storage_provider_register_serializer(
		&m_block_storage_provider,
		TS_RPC_ENCODING_PACKED_C,
		packedc_block_storage_serializer_instance());

	standalone_service_context::set_rpc_interface(rpc_iface);
}

void block_storage_service_context::do_deinit()
{
	block_storage_provider_deinit(&m_block_storage_provider);
	partitioned_block_store_deinit(&m_partitioned_block_store);
	ram_block_store_deinit(&m_ram_block_store);
}
