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
	struct rpc_caller_session *nv_storage_session;

} smm_gateway_instance;


struct rpc_service_interface *smm_gateway_create(uint32_t owner_id)
{
	service_locator_envinit();

	/* todo - add option to use configurable service location */
	smm_gateway_instance.nv_storage_service_context =
		service_locator_query(SMM_GATEWAY_NV_STORE_SN);

	if (!smm_gateway_instance.nv_storage_service_context)
		return NULL;

	smm_gateway_instance.nv_storage_session = service_context_open(
		smm_gateway_instance.nv_storage_service_context);

	if (!smm_gateway_instance.nv_storage_session)
		return NULL;

	/* Initialize a storage client to access the remote NV store */
	struct storage_backend *persistent_backend = secure_storage_client_init(
		&smm_gateway_instance.nv_store_client,
		smm_gateway_instance.nv_storage_session);
	if (!persistent_backend)
		return NULL;

	/* Initialize the volatile storage backend */
	struct storage_backend *volatile_backend  = mock_store_init(
		&smm_gateway_instance.volatile_store);
	if (!volatile_backend)
		return NULL;

	/* Initialize the smm_variable service provider */
	struct rpc_service_interface *service_iface = smm_variable_provider_init(
		&smm_gateway_instance.smm_variable_provider,
 		owner_id,
		SMM_GATEWAY_MAX_UEFI_VARIABLES,
		persistent_backend,
		volatile_backend);

	return service_iface;
}
