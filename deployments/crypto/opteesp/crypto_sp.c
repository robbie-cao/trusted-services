// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 */


#include <rpc/ffarpc/endpoint/ffarpc_call_ep.h>
#include <service/secure_storage/factory/storage_factory.h>
#include <service/crypto/provider/crypto_provider.h>
#include <service/crypto/provider/serializer/protobuf/pb_crypto_provider_serializer.h>
#include <service/crypto/provider/serializer/packed-c/packedc_crypto_provider_serializer.h>
#include <service/crypto/backend/mbedcrypto/mbedcrypto_backend.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <config/ramstore/config_ramstore.h>
#include <config/loader/sp/sp_config_loader.h>
#include <ffa_api.h>
#include <sp_api.h>
#include <sp_messaging.h>
#include <sp_rxtx.h>
#include <trace.h>


uint16_t own_id = 0; /* !!Needs refactoring as parameter to ffarpc_caller_init */


static int sp_init(uint16_t *own_sp_id);

void __noreturn sp_main(struct ffa_init_info *init_info)
{
	struct crypto_provider crypto_provider;
	struct ffa_call_ep ffarpc_call_ep;
	struct rpc_interface *crypto_iface;
	struct sp_msg req_msg = { 0 };
	struct sp_msg resp_msg = { 0 };
	struct storage_backend *storage_backend;

	/* Boot phase */
	if (sp_init(&own_id) != 0) goto fatal_error;

	config_ramstore_init();
	sp_config_load(init_info);

	/* Create a storage backend for persistent key storage - prefer ITS */
	storage_backend = storage_factory_create(storage_factory_security_class_INTERNAL_TRUSTED);
	if (!storage_backend) goto fatal_error;

	/* Initialize the crypto service */
	crypto_iface = NULL;

    if (mbedcrypto_backend_init(storage_backend, 0) == PSA_SUCCESS) {

        crypto_iface = crypto_provider_init(&crypto_provider);

		crypto_provider_register_serializer(&crypto_provider,
                    TS_RPC_ENCODING_PROTOBUF, pb_crypto_provider_serializer_instance());

		crypto_provider_register_serializer(&crypto_provider,
                    TS_RPC_ENCODING_PACKED_C, packedc_crypto_provider_serializer_instance());
	}

	ffa_call_ep_init(&ffarpc_call_ep, crypto_iface);

	/* End of boot phase */
	sp_msg_wait(&req_msg);

	while (1) {
		ffa_call_ep_receive(&ffarpc_call_ep, &req_msg, &resp_msg);

		resp_msg.source_id = req_msg.destination_id;
		resp_msg.destination_id = req_msg.source_id;

		sp_msg_send_direct_resp(&resp_msg, &req_msg);
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

static int sp_init(uint16_t *own_sp_id)
{
	int status = -1;
	ffa_result ffa_res;
	sp_result sp_res;
	static uint8_t tx_buffer[4096] __aligned(4096);
	static uint8_t rx_buffer[4096] __aligned(4096);

	sp_res = sp_rxtx_buffer_map(tx_buffer, rx_buffer, sizeof(rx_buffer));
	if (sp_res == SP_RESULT_OK) {
		ffa_res = ffa_id_get(own_sp_id);
		if (ffa_res == FFA_OK) {
			status = 0;
		}
	}

	return status;
}
