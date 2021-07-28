/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <psa/crypto.h>
#include "psa_crypto_client.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <protocols/service/crypto/packed-c/purge_key.h>


psa_status_t psa_purge_key(psa_key_id_t key)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_purge_key_in req_msg;
	size_t req_len = sizeof(struct ts_crypto_purge_key_in);

	if (psa_crypto_client_instance.init_status != PSA_SUCCESS)
		return psa_crypto_client_instance.init_status;

	req_msg.id = key;

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.base.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;

		memcpy(req_buf, &req_msg, req_len);

		psa_crypto_client_instance.base.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.base.caller, call_handle,
						TS_CRYPTO_OPCODE_PURGE_KEY, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.base.rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

		rpc_caller_end(psa_crypto_client_instance.base.caller, call_handle);
	}

	return psa_status;
}
