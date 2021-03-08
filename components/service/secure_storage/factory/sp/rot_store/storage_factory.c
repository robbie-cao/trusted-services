/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/**
 * A storage factory that creates storage backends that may be used
 * to access a secure storage partition from a separate SP within the
 * device RoT.  Defaults to using PSA storage partitions if no runtime
 * configuration overrides the target service endpoint to use.  If multiple
 * candidate storage SPs are available, the one that matches the
 * requested storage class is used.  The availability of Internal Trusted
 * and Protected stores will depend on the platform.
 */
#include <rpc/ffarpc/caller/sp/ffarpc_caller.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>
#include <service/secure_storage/backend/null_store/null_store.h>
#include <service/secure_storage/factory/storage_factory.h>
#include <ffa_api.h>
#include <stdbool.h>
#include <stddef.h>

/* Defaults to using PSA storage partitions if no external configuration specified */
#define ITS_STORE_UUID_BYTES \
	{ 0xdc, 0x1e, 0xef, 0x48, 0xb1, 0x7a, 0x4c, 0xcf, \
	  0xac, 0x8b, 0xdf, 0xcf, 0xf7, 0x71, 0x1b, 0x14 }

#define PS_STORE_UUID_BYTES \
	{ 0x75, 0x1b, 0xf8, 0x01, 0x3d, 0xde, 0x47, 0x68, \
	  0xa5, 0x14, 0x0f, 0x10, 0xae, 0xed, 0x17, 0x90 }

#define MAX_CANDIDATE_UUIDS		(2)

static const uint8_t default_internal_store_uuid[] = ITS_STORE_UUID_BYTES;
static const uint8_t default_protected_store_uuid[] = PS_STORE_UUID_BYTES;

/* The storage backed specialization constructed by this factory */
struct rot_store
{
	struct secure_storage_client secure_storage_client;
	struct ffarpc_caller ffarpc_caller;
	bool in_use;
};

/* Only supports construction of a single instance */
static struct rot_store backend_instance = { .in_use = false };

/* Used on failure if no association with a storage provider is established */
static struct null_store null_store;

static int select_candidate_uuids(const uint8_t *candidates[],
							int max_candidates,
							enum storage_factory_security_class security_class);


struct storage_backend *storage_factory_create(
			enum storage_factory_security_class security_class)
{
	struct rpc_caller *storage_caller;
	uint16_t storage_sp_ids[1];
	struct rot_store *new_backend = &backend_instance;
	const uint8_t *candidate_uuids[MAX_CANDIDATE_UUIDS];
	int num_candidate_uuids = select_candidate_uuids(candidate_uuids,
										MAX_CANDIDATE_UUIDS, security_class);

	struct storage_backend *result = NULL;

	if (num_candidate_uuids && !new_backend->in_use) {

		storage_caller = ffarpc_caller_init(&new_backend->ffarpc_caller);

		for (int i = 0; i < num_candidate_uuids; i++) {

			/* Try discovering candidate endpoints in preference order */
			if (ffarpc_caller_discover(candidate_uuids[i], storage_sp_ids,
									sizeof(storage_sp_ids)/sizeof(uint16_t))) {

				if (ffarpc_caller_open(&new_backend->ffarpc_caller, storage_sp_ids[0], 0) == 0) {

					result = secure_storage_client_init(&new_backend->secure_storage_client,
														storage_caller);
				}

				break;
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

static int select_candidate_uuids(const uint8_t *candidates[],
							int max_candidates,
							enum storage_factory_security_class security_class)
{
	/* Runtime configuration not yet supported so fallback to using default UUIDs */
	int num_candidates = 0;

	if (max_candidates >= 2) {

		if (security_class == storage_factory_security_class_INTERNAL_TRUSTED) {

			candidates[0] = default_internal_store_uuid;
			candidates[1] = default_protected_store_uuid;
		}
		else {

			candidates[0] = default_protected_store_uuid;
			candidates[1] = default_internal_store_uuid;
		}

		num_candidates = 2;
	}

	return num_candidates;
}