/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <rpc/common/endpoint/rpc_interface.h>
#include <service/attestation/provider/attest_provider.h>
#include <service/attestation/provider/serializer/packed-c/packedc_attest_provider_serializer.h>
#include <service/crypto/provider/crypto_provider.h>
#include <service/crypto/provider/serializer/protobuf/pb_crypto_provider_serializer.h>
#include <service/crypto/provider/serializer/packed-c/packedc_crypto_provider_serializer.h>
#include <components/service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h>

/* Not needed once proxy backends added */
#include <service/attestation/claims/claims_register.h>
#include <service/attestation/claims/sources/event_log/event_log_claim_source.h>
#include <service/attestation/claims/sources/boot_seed_generator/boot_seed_generator.h>
#include <service/attestation/claims/sources/null_lifecycle/null_lifecycle_claim_source.h>
#include <service/attestation/claims/sources/instance_id/instance_id_claim_source.h>
#include <service/secure_storage/backend/secure_flash_store/secure_flash_store.h>
#include <service/crypto/backend/mbedcrypto/mbedcrypto_backend.h>


/* A shared storage backend - should be removed when proxy backends are added */
static struct storage_backend *shared_storage_backend = NULL;


struct rpc_interface *attest_proxy_create(void)
{
	struct rpc_interface *attest_iface;
	struct claim_source *claim_source;

	/* Static objects for proxy instance */
	static struct attest_provider attest_provider;

	/* Claim sources for deployment */
	static struct event_log_claim_source event_log_claim_source;
	static struct boot_seed_generator boot_seed_claim_source;
	static struct null_lifecycle_claim_source lifecycle_claim_source;
	static struct instance_id_claim_source instance_id_claim_source;

	/* Register claim sources for deployment */
	claims_register_init();

	/* Boot measurement claim source */
	claim_source = event_log_claim_source_init_from_config(&event_log_claim_source);
	claims_register_add_claim_source(CLAIM_CATEGORY_BOOT_MEASUREMENT, claim_source);

	/* Boot seed claim source */
	claim_source = boot_seed_generator_init(&boot_seed_claim_source);
	claims_register_add_claim_source(CLAIM_CATEGORY_DEVICE, claim_source);

	/* Lifecycle state claim source */
	claim_source = null_lifecycle_claim_source_init(&lifecycle_claim_source);
	claims_register_add_claim_source(CLAIM_CATEGORY_DEVICE, claim_source);

	/* Instance ID claim source */
	claim_source = instance_id_claim_source_init(&instance_id_claim_source);
	claims_register_add_claim_source(CLAIM_CATEGORY_DEVICE, claim_source);

	/* Initialize the service provider */
	attest_iface = attest_provider_init(&attest_provider, ATTEST_KEY_MNGR_VOLATILE_IAK);

	attest_provider_register_serializer(&attest_provider,
		TS_RPC_ENCODING_PACKED_C, packedc_attest_provider_serializer_instance());

	return attest_iface;
}

struct rpc_interface *crypto_proxy_create(void)
{
	struct rpc_interface *crypto_iface = NULL;

	/* Static objects for proxy instance */
	static struct crypto_provider crypto_provider;

	if (mbedcrypto_backend_init(shared_storage_backend, 0) == PSA_SUCCESS) {

		crypto_iface = crypto_provider_init(&crypto_provider);

		crypto_provider_register_serializer(&crypto_provider,
					TS_RPC_ENCODING_PROTOBUF, pb_crypto_provider_serializer_instance());

		crypto_provider_register_serializer(&crypto_provider,
					TS_RPC_ENCODING_PACKED_C, packedc_crypto_provider_serializer_instance());
	}

	return crypto_iface;
}

struct rpc_interface *ps_proxy_create(void)
{
	if (!shared_storage_backend)    shared_storage_backend = sfs_init();

	static struct secure_storage_provider ps_provider;

	return secure_storage_provider_init(&ps_provider, shared_storage_backend);
}

struct rpc_interface *its_proxy_create(void)
{
	if (!shared_storage_backend)    shared_storage_backend = sfs_init();

	static struct secure_storage_provider its_provider;

	return secure_storage_provider_init(&its_provider, shared_storage_backend);
}
