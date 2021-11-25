/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PACKEDC_CRYPTO_CALLER_COPY_KEY_H
#define PACKEDC_CRYPTO_CALLER_COPY_KEY_H

#include <string.h>
#include <psa/crypto.h>
#include <service/common/client/service_client.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <protocols/service/crypto/packed-c/copy_key.h>
#include "crypto_caller_key_attributes.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline psa_status_t crypto_caller_copy_key(struct service_client *context,
	psa_key_id_t source_key,
	const psa_key_attributes_t *attributes,
	psa_key_id_t *target_key)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_copy_key_in req_msg;
	size_t req_len = sizeof(struct ts_crypto_copy_key_in);

	/* Set default outputs for failure case */
	*target_key = 0;

	req_msg.source_key_id = source_key;
	packedc_crypto_caller_translate_key_attributes_to_proto(&req_msg.attributes, attributes);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(context->caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		rpc_opstatus_t opstatus;

		memcpy(req_buf, &req_msg, req_len);

		context->rpc_status = rpc_caller_invoke(context->caller, call_handle,
			TS_CRYPTO_OPCODE_COPY_KEY, &opstatus, &resp_buf, &resp_len);

		if (context->rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				if (resp_len >= sizeof(struct ts_crypto_copy_key_out)) {

					struct ts_crypto_copy_key_out resp_msg;
					memcpy(&resp_msg, resp_buf, sizeof(struct ts_crypto_copy_key_out));
					*target_key = resp_msg.target_key_id;
				}
				else {
					/* Failed to decode response message */
					psa_status = PSA_ERROR_GENERIC_ERROR;
				}
			}
		}

		rpc_caller_end(context->caller, call_handle);
	}

	return psa_status;
}

#ifdef __cplusplus
}
#endif

#endif /* PACKEDC_CRYPTO_CALLER_COPY_KEY_H */
