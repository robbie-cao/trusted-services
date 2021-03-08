/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <rpc/ffarpc/caller/sp/ffarpc_caller.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <service/secure_storage/backend/secure_flash_store/secure_flash_store.h>
#include <service/secure_storage/factory/storage_factory.h>

/**
 * \brief Constructs a secure flash store
 *
 * Can be used as a storage backend in any environment.  However
 * it doesn't actually provide persistent flash storage without
 * platform specific hardware.
 */
struct storage_backend *storage_factory_create(
			enum storage_factory_security_class security_class)
{
	(void)security_class;
	return sfs_init();
}

void storage_factory_destroy(struct storage_backend *backend)
{
	(void)backend;
}
