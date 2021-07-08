/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <psa/crypto.h>
#include "psa_crypto_client.h"
#include "psa_crypto_client_key_attributes.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <protocols/service/crypto/packed-c/get_key_attributes.h>


psa_status_t psa_get_key_attributes(psa_key_id_t key,
	psa_key_attributes_t *attributes)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_get_key_attributes_in req_msg;
	size_t req_len = sizeof(struct ts_crypto_get_key_attributes_in);

	if (psa_crypto_client_instance.init_status != PSA_SUCCESS)
		return psa_crypto_client_instance.init_status;

	req_msg.id = key;

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;

		memcpy(req_buf, &req_msg, req_len);

		psa_crypto_client_instance.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.caller, call_handle,
						TS_CRYPTO_OPCODE_GET_KEY_ATTRIBUTES, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				if (resp_len >= sizeof(struct ts_crypto_get_key_attributes_out)) {

					struct ts_crypto_get_key_attributes_out resp_msg;
					memcpy(&resp_msg, resp_buf, sizeof(struct ts_crypto_get_key_attributes_out));
					psa_crypto_client_translate_key_attributes_from_proto(
						attributes, &resp_msg.attributes);
				}
				else {
					/* Failed to decode response message */
					psa_status = PSA_ERROR_GENERIC_ERROR;
				}
			}
		}

		rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
	}

	return psa_status;
}
