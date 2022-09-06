/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <rpc/common/endpoint/rpc_interface.h>
#include <service/attestation/provider/attest_provider.h>
#include <service/attestation/provider/serializer/packed-c/packedc_attest_provider_serializer.h>
#include <service/crypto/factory/crypto_provider_factory.h>
#include <service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h>

/* Stub backends */
#include <service/crypto/backend/stub/stub_crypto_backend.h>
#include <service/secure_storage/backend/mock_store/mock_store.h>

struct rpc_interface *attest_proxy_create(void)
{
	struct rpc_interface *attest_iface;

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
	static struct mock_store ps_backend;
	static struct secure_storage_provider ps_provider;

	struct storage_backend *backend = mock_store_init(&ps_backend);

	return secure_storage_provider_init(&ps_provider, backend);
}

struct rpc_interface *its_proxy_create(void)
{
	static struct mock_store its_backend;
	static struct secure_storage_provider its_provider;

	struct storage_backend *backend = mock_store_init(&its_backend);

	return secure_storage_provider_init(&its_provider, backend);
}
