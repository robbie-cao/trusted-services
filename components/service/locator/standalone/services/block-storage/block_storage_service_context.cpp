/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cstring>
#include "service/block_storage/provider/serializer/packed-c/packedc_block_storage_serializer.h"
#include "service/block_storage/factory/ref_ram_gpt/block_store_factory.h"
#include "block_storage_service_context.h"

block_storage_service_context::block_storage_service_context(const char *sn) :
	standalone_service_context(sn),
	m_block_storage_provider(),
	m_block_store(NULL)
{

}

block_storage_service_context::~block_storage_service_context()
{

}

void block_storage_service_context::do_init()
{
	/* Create backend block store */
	m_block_store = ref_ram_gpt_block_store_factory_create();
	assert(m_block_store);

	/* Initialise the block storage service provider */
	struct rpc_interface *rpc_iface = block_storage_provider_init(
		&m_block_storage_provider,
		m_block_store);
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
	ref_ram_gpt_block_store_factory_destroy(m_block_store);
}
