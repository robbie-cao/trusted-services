// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 */

#include "rpc/ffarpc/endpoint/ffarpc_call_ep.h"
#include "service/secure_storage/factory/storage_factory.h"
#include "service/crypto/factory/crypto_provider_factory.h"
#include "service/crypto/backend/mbedcrypto/mbedcrypto_backend.h"
#include "protocols/rpc/common/packed-c/status.h"
#include "config/ramstore/config_ramstore.h"
#include "config/loader/sp/sp_config_loader.h"
#include "sp_api.h"
#include "sp_discovery.h"
#include "sp_messaging.h"
#include "sp_rxtx.h"
#include "trace.h"

static bool sp_init(uint16_t *own_sp_id);

void __noreturn sp_main(struct ffa_init_info *init_info)
{
	struct crypto_provider *crypto_provider = NULL;
	struct ffa_call_ep ffarpc_call_ep = { 0 };
	struct rpc_interface *crypto_iface = NULL;
	struct sp_msg req_msg = { 0 };
	struct sp_msg resp_msg = { 0 };
	struct storage_backend *storage_backend = NULL;
	uint16_t own_id = 0;
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	sp_result result = SP_RESULT_INTERNAL_ERROR;

	/* Boot phase */
	if (!sp_init(&own_id)) {
		EMSG("Failed to init SP");
		goto fatal_error;
	}

	config_ramstore_init();

	if (!sp_config_load(init_info)) {
		EMSG("Failed to load SP config");
		goto fatal_error;
	}

	/* Create a storage backend for persistent key storage - prefer ITS */
	storage_backend = storage_factory_create(storage_factory_security_class_INTERNAL_TRUSTED);
	if (!storage_backend) {
		EMSG("Failed to create storage factory");
		goto fatal_error;
	}

	/* Initialize the crypto service */
	psa_status = mbedcrypto_backend_init(storage_backend, 0);
	if (psa_status != PSA_SUCCESS) {
		EMSG("Failed to init Mbed TLS backend: %d", psa_status);
		goto fatal_error;
	}

	crypto_provider = crypto_provider_factory_create();
	if (!crypto_provider) {
		EMSG("Failed to create crypto provider factory");
		goto fatal_error;
	}

	crypto_iface = service_provider_get_rpc_interface(&crypto_provider->base_provider);
	if (!crypto_iface) {
		EMSG("Failed to create service provider RPC interface");
		goto fatal_error;
	}

	ffa_call_ep_init(&ffarpc_call_ep, crypto_iface, own_id);

	/* End of boot phase */
	result = sp_msg_wait(&req_msg);
	if (result != SP_RESULT_OK) {
		EMSG("Failed to send message wait %d", result);
		goto fatal_error;
	}

	while (1) {
		ffa_call_ep_receive(&ffarpc_call_ep, &req_msg, &resp_msg);

		result = sp_msg_send_direct_resp(&resp_msg, &req_msg);
		if (result != SP_RESULT_OK) {
			EMSG("Failed to send direct response %d", result);
			result = sp_msg_wait(&req_msg);
			if (result != SP_RESULT_OK) {
				EMSG("Failed to send message wait %d", result);
				goto fatal_error;
			}
		}
	}

fatal_error:
	/* SP is not viable */
	EMSG("Crypto SP error");
	while (1) {}
}

void sp_interrupt_handler(uint32_t interrupt_id)
{
	(void)interrupt_id;
}

static bool sp_init(uint16_t *own_id)
{
	sp_result sp_res = SP_RESULT_INTERNAL_ERROR;
	static uint8_t tx_buffer[4096] __aligned(4096);
	static uint8_t rx_buffer[4096] __aligned(4096);

	sp_res = sp_rxtx_buffer_map(tx_buffer, rx_buffer, sizeof(rx_buffer));
	if (sp_res != SP_RESULT_OK) {
		EMSG("Failed to map RXTX buffers: %d", sp_res);
		return false;
	}

	sp_res = sp_discovery_own_id_get(own_id);
	if (sp_res != SP_RESULT_OK) {
		EMSG("Failed to query own ID: %d", sp_res);
		return false;
	}

	return true;
}
