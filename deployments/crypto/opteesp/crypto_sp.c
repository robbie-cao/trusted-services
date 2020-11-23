// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 */

#include <rpc/ffarpc/caller/sp/ffarpc_caller.h>
#include <rpc/ffarpc/endpoint/ffarpc_call_ep.h>
#include <rpc/dummy/dummy_caller.h>
#include <service/secure_storage/client/psa/its/its_client.h>
#include <service/crypto/provider/mbedcrypto/crypto_provider.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <ffa_api.h>
#include <sp_api.h>
#include <sp_rxtx.h>
#include <trace.h>


#define SP_STORAGE_UUID_BYTES \
	{ 0x48, 0xef, 0x1e, 0xdc, 0x7a, 0xb1, 0xcf, 0x4c, \
	  0xac, 0x8b, 0xdf, 0xcf, 0xf7, 0x71, 0x1b, 0x14, }

uint16_t own_id = 0; /* !!Needs refactoring as parameter to ffarpc_caller_init */
static const uint8_t storage_uuid[] = SP_STORAGE_UUID_BYTES;


static int sp_init(uint16_t *own_sp_id);

void __noreturn sp_main(struct ffa_init_info *init_info)
{
	struct mbed_crypto_provider crypto_provider;
	struct ffa_call_ep ffarpc_call_ep;
	struct call_ep *crypto_ep;
	struct ffarpc_caller ffarpc_caller;
	struct dummy_caller dummy_caller;
	struct rpc_caller *storage_caller;
	struct ffa_direct_msg req_msg;
	uint16_t storage_sp_ids[1];

	/* Boot */
	(void) init_info;

	if (sp_init(&own_id) != 0) goto fatal_error;

	/* Establish RPC session with secure storage SP */
	storage_caller = ffarpc_caller_init(&ffarpc_caller);

	if (!ffarpc_caller_discover(storage_uuid, storage_sp_ids, sizeof(storage_sp_ids)/sizeof(uint16_t)) ||
		ffarpc_caller_open(&ffarpc_caller, storage_sp_ids[0])) {
		/*
		 * Failed to establish session.  To allow the crypto service
		 * to still be initialized, albeit with no persistent storage,
		 * initialise a dummy_caller that will safely
		 * handle rpc requests but will report an error.
		 */
		storage_caller = dummy_caller_init(&dummy_caller,
                                TS_RPC_CALL_ACCEPTED, PSA_ERROR_STORAGE_FAILURE);
	}

	/* Initialize the crypto service */
	crypto_ep = mbed_crypto_provider_init(&crypto_provider, storage_caller);
	ffa_call_ep_init(&ffarpc_call_ep, crypto_ep);

 	/* End of boot phase */
	ffa_msg_wait(&req_msg);

	while (1) {
		if (req_msg.function_id == FFA_MSG_SEND_DIRECT_REQ_32) {

			struct ffa_direct_msg resp_msg;

			ffa_call_ep_receive(&ffarpc_call_ep, &req_msg, &resp_msg);

			ffa_msg_send_direct_resp(req_msg.destination_id,
					req_msg.source_id, resp_msg.args[0], resp_msg.args[1],
					resp_msg.args[2], resp_msg.args[3], resp_msg.args[4],
					&req_msg);
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
