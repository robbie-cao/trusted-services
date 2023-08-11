/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2023, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <psa/sid.h>
#include <rpc/common/endpoint/rpc_interface.h>
#include <rpc/rss_comms/caller/sp/rss_comms_caller.h>
#include <service/attestation/provider/attest_provider.h>
#include <service/attestation/provider/serializer/packed-c/packedc_attest_provider_serializer.h>
#include <service/crypto/factory/crypto_provider_factory.h>
#include <service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h>
#include <trace.h>

struct rss_comms_caller rss_comms;

/* IPC backends */
#include <service/secure_storage/backend/secure_storage_ipc/secure_storage_ipc.h>
/* Stub backends */
#include <service/crypto/backend/stub/stub_crypto_backend.h>
#include <service/secure_storage/backend/mock_store/mock_store.h>

struct rpc_interface *attest_proxy_create(void)
{
	struct rpc_interface *attest_iface = NULL;

	/* Static objects for proxy instance */
	static struct attest_provider attest_provider;

	/* Initialize the service provider */
	attest_iface = attest_provider_init(&attest_provider);

	attest_provider_register_serializer(&attest_provider,
		TS_RPC_ENCODING_PACKED_C, packedc_attest_provider_serializer_instance());

	return attest_iface;
}

struct rpc_interface *crypto_proxy_create(void)
{
	struct rpc_interface *crypto_iface = NULL;
	struct crypto_provider *crypto_provider;

	if (stub_crypto_backend_init() == PSA_SUCCESS) {

		crypto_provider = crypto_provider_factory_create();
		crypto_iface = service_provider_get_rpc_interface(&crypto_provider->base_provider);
	}

	return crypto_iface;
}

struct rpc_interface *ps_proxy_create(void)
{
	struct rpc_interface *rpc_interface = NULL;

	static struct secure_storage_provider ps_provider;
	static struct secure_storage_ipc ps_backend;
	struct rpc_caller *storage_caller;
	struct storage_backend *backend;

	storage_caller = rss_comms_caller_init(&rss_comms);
	if (!storage_caller) {
		EMSG("storage caller is null\n");
		return NULL;
	}
	backend = secure_storage_ipc_init(&ps_backend, &rss_comms.rpc_caller);
	ps_backend.service_handle = TFM_PROTECTED_STORAGE_SERVICE_HANDLE;

	rpc_interface = secure_storage_provider_init(&ps_provider, backend);

	return rpc_interface;
}

struct rpc_interface *its_proxy_create(void)
{
	static struct mock_store its_backend;
	static struct secure_storage_provider its_provider;

	struct storage_backend *backend = mock_store_init(&its_backend);

	return secure_storage_provider_init(&its_provider, backend);
}
