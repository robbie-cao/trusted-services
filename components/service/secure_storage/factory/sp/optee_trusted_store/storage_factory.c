/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/**
 * A storage factory that creates storage backends that communicate with an
 * S-EL1 partition to access trusted storage provided by OPTEE. The S-EL1
 * partition is assumed to host a conventional secure storage provider
 * that can be accessed using the secure storage access protocol.
 * Uses a default UUID to discover the S-EL1 partition if no external
 * configuration overrides this.
 */
#include <rpc/ffarpc/caller/sp/ffarpc_caller.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>
#include <service/secure_storage/backend/null_store/null_store.h>
#include <service/secure_storage/factory/storage_factory.h>
#include <ffa_api.h>
#include "sp_discovery.h"
#include <stdbool.h>
#include <stddef.h>

/* NOTE: this is the ITS partition UUID - should be changed when S-EL1 SP is ready */
#define OPTEE_TRUSTED_STORE_UUID_BYTES \
	{ 0xdc, 0x1e, 0xef, 0x48, 0xb1, 0x7a, 0x4c, 0xcf, \
	  0xac, 0x8b, 0xdf, 0xcf, 0xf7, 0x71, 0x1b, 0x14 }

static const uint8_t default_optee_trusted_store_uuid[] = OPTEE_TRUSTED_STORE_UUID_BYTES;

/* The storage backed specialization constructed by this factory */
struct optee_trusted_store
{
	struct secure_storage_client secure_storage_client;
	struct ffarpc_caller ffarpc_caller;
	bool in_use;
};

/* Only supports construction of a single instance */
static struct optee_trusted_store backend_instance = { .in_use = false };

/* Used on failure if no association with a storage provider is established */
static struct null_store null_store;


struct storage_backend *storage_factory_create(
			enum storage_factory_security_class security_class)
{
	struct rpc_caller *storage_caller;
	uint16_t storage_sp_ids[1];
	struct optee_trusted_store *new_backend = &backend_instance;
	struct storage_backend *result = NULL;

	if (!new_backend->in_use) {
		uint16_t own_id = 0;

		if (sp_discovery_own_id_get(&own_id) != SP_RESULT_OK)
			return NULL;

		storage_caller = ffarpc_caller_init(&new_backend->ffarpc_caller, own_id);
		if (!storage_caller)
			return NULL;

		/* Try discovering candidate endpoints in preference order */
		if (ffarpc_caller_discover(default_optee_trusted_store_uuid, storage_sp_ids,
					   sizeof(storage_sp_ids)/sizeof(uint16_t))) {

			if (ffarpc_caller_open(&new_backend->ffarpc_caller,
					       storage_sp_ids[0], 0) == 0) {

				result = secure_storage_client_init(
					&new_backend->secure_storage_client,
					storage_caller);
			}
		}

		if (!result) {

			/* Failed to discover or open an RPC session with provider */
			ffarpc_caller_deinit(&new_backend->ffarpc_caller);
		}

		new_backend->in_use = (result != NULL);
	}

	if (!result) {

		/**
		 * Errors during SP initialisation can be difficult to handle so
		 * returns a valid storage_backend, albeit one that just returns
		 * an appropriate status code if any methods are called.  This
		 * allows an error to be reported to a requesting client where
		 * it may be easier to handle.
		 */
		result = null_store_init(&null_store);
	}

	return result;
}

void storage_factory_destroy(struct storage_backend *backend)
{
	if (backend) {

		secure_storage_client_deinit(&backend_instance.secure_storage_client);
		ffarpc_caller_deinit(&backend_instance.ffarpc_caller);
		backend_instance.in_use = false;
	}
}
