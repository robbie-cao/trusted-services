/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include <service/smm_variable/provider/smm_variable_provider.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>
#include <service/secure_storage/backend/mock_store/mock_store.h>
#include <service_locator.h>

/* Build-time default configuration */

/* Default to using the Protected Storage SP */
#ifndef SMM_GATEWAY_NV_STORE_SN
#define SMM_GATEWAY_NV_STORE_SN		"sn:ffa:751bf801-3dde-4768-a514-0f10aeed1790:0"
#endif

/* Default maximum number of UEFI variables */
#ifndef SMM_GATEWAY_MAX_UEFI_VARIABLES
#define SMM_GATEWAY_MAX_UEFI_VARIABLES		(40)
#endif

/* The smm_gateway instance - it's a singleton */
static struct smm_gateway
{
	struct smm_variable_provider smm_variable_provider;
	struct secure_storage_client nv_store_client;
	struct mock_store volatile_store;
	struct service_context *nv_storage_service_context;
	rpc_session_handle nv_storage_session_handle;

} smm_gateway_instance;


static struct rpc_caller *locate_nv_store(void)
{
	int status = 0;
	struct rpc_caller *caller = NULL;

	/* todo - add option to use configurable service location */
	smm_gateway_instance.nv_storage_service_context =
		service_locator_query(SMM_GATEWAY_NV_STORE_SN, &status);

	if (smm_gateway_instance.nv_storage_service_context) {

		smm_gateway_instance.nv_storage_session_handle = service_context_open(
			smm_gateway_instance.nv_storage_service_context,
			TS_RPC_ENCODING_PACKED_C,
			&caller);
	}

	return caller;
}

struct rpc_interface *smm_gateway_create(uint32_t owner_id)
{
	service_locator_init();

	/* Initialize a storage client to access the remote NV store */
	struct rpc_caller *nv_store_caller = locate_nv_store();
	struct storage_backend *persistent_backend = secure_storage_client_init(
		&smm_gateway_instance.nv_store_client,
		nv_store_caller);

	/* Initialize the volatile storage backend */
	struct storage_backend *volatile_backend  = mock_store_init(
		&smm_gateway_instance.volatile_store);

	/* Initialize the smm_variable service provider */
	struct rpc_interface *service_iface = smm_variable_provider_init(
		&smm_gateway_instance.smm_variable_provider,
 		owner_id,
		SMM_GATEWAY_MAX_UEFI_VARIABLES,
		persistent_backend,
		volatile_backend);

	return service_iface;
}
