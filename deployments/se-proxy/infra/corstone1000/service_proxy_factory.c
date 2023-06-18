/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2023, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <psa/sid.h>
#include <rpc/common/endpoint/rpc_interface.h>
#include <rpc/psa_ipc/caller/sp/psa_ipc_caller.h>
#include <service/attestation/provider/attest_provider.h>
#include <service/attestation/provider/serializer/packed-c/packedc_attest_provider_serializer.h>
#include <service/crypto/factory/crypto_provider_factory.h>
#include <service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h>
#include <trace.h>
#include <service/capsule_update/provider/capsule_update_provider.h>

/* backends */
#include <service/crypto/backend/psa_ipc/crypto_ipc_backend.h>
#include <service/secure_storage/backend/secure_storage_ipc/secure_storage_ipc.h>
#include <service/attestation/client/psa/iat_client.h>

struct psa_ipc_caller psa_ipc;

struct rpc_interface *attest_proxy_create(void)
{
	struct rpc_interface *attest_iface;
	struct rpc_caller *attest_caller;

	/* Static objects for proxy instance */
	static struct attest_provider attest_provider;

	attest_caller = psa_ipc_caller_init(&psa_ipc);
	if (!attest_caller)
		return NULL;

	/* Initialize the service provider */
	attest_iface = attest_provider_init(&attest_provider);
	psa_iat_client_init(&psa_ipc.rpc_caller);

	attest_provider_register_serializer(&attest_provider,
		TS_RPC_ENCODING_PACKED_C, packedc_attest_provider_serializer_instance());

	return attest_iface;
}

struct rpc_interface *crypto_proxy_create(void)
{
	struct rpc_interface *crypto_iface = NULL;
	struct crypto_provider *crypto_provider;
	struct rpc_caller *crypto_caller;

	crypto_caller = psa_ipc_caller_init(&psa_ipc);
	if (!crypto_caller)
		return NULL;

	if (crypto_ipc_backend_init(&psa_ipc.rpc_caller) != PSA_SUCCESS)
		return NULL;

	crypto_provider = crypto_provider_factory_create();
	crypto_iface = service_provider_get_rpc_interface(&crypto_provider->base_provider);

	return crypto_iface;
}

struct rpc_interface *ps_proxy_create(void)
{
	static struct secure_storage_provider ps_provider;
	static struct secure_storage_ipc ps_backend;
	struct rpc_caller *storage_caller;
	struct storage_backend *backend;

	storage_caller = psa_ipc_caller_init(&psa_ipc);
	if (!storage_caller)
		return NULL;
	backend = secure_storage_ipc_init(&ps_backend, &psa_ipc.rpc_caller);
	ps_backend.service_handle = TFM_PROTECTED_STORAGE_SERVICE_HANDLE;

	return secure_storage_provider_init(&ps_provider, backend);
}

struct rpc_interface *its_proxy_create(void)
{
	static struct secure_storage_provider its_provider;
	static struct secure_storage_ipc its_backend;
	struct rpc_caller *storage_caller;
	struct storage_backend *backend;

	storage_caller = psa_ipc_caller_init(&psa_ipc);
	if (!storage_caller)
		return NULL;
	backend = secure_storage_ipc_init(&its_backend, &psa_ipc.rpc_caller);
	its_backend.service_handle = TFM_INTERNAL_TRUSTED_STORAGE_SERVICE_HANDLE;

	return secure_storage_provider_init(&its_provider, backend);
}

struct rpc_interface *capsule_update_proxy_create(void)
{
	static struct capsule_update_provider capsule_update_provider;
	static struct rpc_caller *capsule_update_caller;

	capsule_update_caller = psa_ipc_caller_init(&psa_ipc);

	if (!capsule_update_caller)
	return NULL;

	capsule_update_provider.client.caller = capsule_update_caller;

	return capsule_update_provider_init(&capsule_update_provider);
}

