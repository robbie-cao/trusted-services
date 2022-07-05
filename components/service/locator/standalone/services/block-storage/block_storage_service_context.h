/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STANDALONE_BLOCK_STORAGE_SERVICE_CONTEXT_H
#define STANDALONE_BLOCK_STORAGE_SERVICE_CONTEXT_H

#include <service/locator/standalone/standalone_service_context.h>
#include <rpc/direct/direct_caller.h>
#include <service/block_storage/provider/block_storage_provider.h>
#include <service/block_storage/block_store/device/ram/ram_block_store.h>
#include <service/block_storage/block_store/partitioned/partitioned_block_store.h>

class block_storage_service_context : public standalone_service_context
{
public:
	block_storage_service_context(const char *sn);
	virtual ~block_storage_service_context();

private:

	void do_init();
	void do_deinit();

	struct block_storage_provider m_block_storage_provider;
	struct ram_block_store m_ram_block_store;
	struct partitioned_block_store m_partitioned_block_store;
};

#endif /* STANDALONE_BLOCK_STORAGE_SERVICE_CONTEXT_H */
