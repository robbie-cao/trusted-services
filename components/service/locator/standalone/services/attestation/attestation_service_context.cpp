/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "attestation_service_context.h"
#include <service/attestation/provider/serializer/packed-c/packedc_attest_provider_serializer.h>
#include <service/attestation/claims/claims_register.h>
#include <service/attestation/claims/sources/event_log/event_log_claim_source.h>
#include <service/attestation/claims/sources/event_log/mock/mock_event_log.h>
#include <config/ramstore/config_ramstore.h>
#include <config/interface/config_store.h>
#include <config/interface/config_blob.h>
#include <psa/crypto.h>

attestation_service_context::attestation_service_context(const char *sn) :
	standalone_service_context(sn),
	m_attest_provider(),
	m_event_log_claim_source(),
	m_boot_seed_claim_source(),
	m_lifecycle_claim_source(),
	m_instance_id_claim_source()
{

}

attestation_service_context::~attestation_service_context()
{

}

void attestation_service_context::do_init()
{
	struct claim_source *claim_source;
	struct config_blob event_log_blob;

	/* For the standalone attestation service deployment, the
	 * mbedcrypto library is used directly.  Note that psa_crypto_init()
	 * is allowed to be called multiple times.
	 */
	psa_crypto_init();

	/**
	 * Initialize the config_store and load dynamic parameters.  For
	 * the attestation service provider, the TPM event log is expected
	 * to be loaded as a dynamic parameter.
	 */
	config_ramstore_init();

	event_log_blob.data = mock_event_log_start();
	event_log_blob.data_len = mock_event_log_size();

	config_store_add(CONFIG_CLASSIFIER_BLOB,
		"EVENT_LOG", 0, &event_log_blob, sizeof(event_log_blob));

	/**
	 * Initialize and register claims sources to define the view of
	 * the device reflected by the attestation service.  On a real
	 * device, the set of claim sources will be deployment specific
	 * to accommodate specific device architecture and product
	 * variations.
	 */
	claims_register_init();

	/* Boot measurement claim source */
	claim_source = event_log_claim_source_init_from_config(&m_event_log_claim_source);
	claims_register_add_claim_source(CLAIM_CATEGORY_BOOT_MEASUREMENT, claim_source);

	/* Boot seed claim source */
	claim_source = boot_seed_generator_init(&m_boot_seed_claim_source);
	claims_register_add_claim_source(CLAIM_CATEGORY_DEVICE, claim_source);

	/* Lifecycle state claim source */
	claim_source = null_lifecycle_claim_source_init(&m_lifecycle_claim_source);
	claims_register_add_claim_source(CLAIM_CATEGORY_DEVICE, claim_source);

	/* Instance ID claim source */
	claim_source = instance_id_claim_source_init(&m_instance_id_claim_source);
	claims_register_add_claim_source(CLAIM_CATEGORY_DEVICE, claim_source);

	/* Initialize the attestation service provider */
	struct rpc_interface *attest_ep =
		attest_provider_init(&m_attest_provider, ATTEST_KEY_MNGR_VOLATILE_IAK);

	attest_provider_register_serializer(&m_attest_provider,
		TS_RPC_ENCODING_PACKED_C, packedc_attest_provider_serializer_instance());

	standalone_service_context::set_rpc_interface(attest_ep);
}

void attestation_service_context::do_deinit()
{
	attest_provider_deinit(&m_attest_provider);
	claims_register_deinit();
	config_ramstore_deinit();
}
