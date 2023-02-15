/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ps_service_context.h"
#include "service/block_storage/factory/client/block_store_factory.h"
#include "service/secure_storage/backend/secure_flash_store/secure_flash_store.h"
#include "media/disk/guid.h"

ps_service_context::ps_service_context(const char *sn) :
	standalone_service_context(sn),
	m_storage_provider(),
	m_sfs_flash_adapter(),
	m_block_store(NULL)
{

}

ps_service_context::~ps_service_context()
{

}

void ps_service_context::do_init()
{
	struct uuid_octets guid;
	const struct sfs_flash_info_t *flash_info = NULL;

	uuid_parse_to_octets(DISK_GUID_UNIQUE_PARTITION_PSA_PS, guid.octets, sizeof(guid.octets));

	m_block_store = client_block_store_factory_create(
		"sn:trustedfirmware.org:block-storage:0");

	psa_status_t status = sfs_flash_block_store_adapter_init(
		&m_sfs_flash_adapter,
		0,
		m_block_store,
		&guid,
		MIN_FLASH_BLOCK_SIZE,
		MAX_NUM_FILES,
		&flash_info);

	if (status == PSA_SUCCESS) {

		struct storage_backend *storage_backend = sfs_init(flash_info);
		struct rpc_interface *storage_ep = secure_storage_provider_init(
			&m_storage_provider, storage_backend);

		standalone_service_context::set_rpc_interface(storage_ep);
	}
}

void ps_service_context::do_deinit()
{
	secure_storage_provider_deinit(&m_storage_provider);
	client_block_store_factory_destroy(m_block_store);
	m_block_store = NULL;
}
