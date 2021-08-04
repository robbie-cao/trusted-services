/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PACKEDC_CRYPTO_CALLER_EXPORT_PUBLIC_KEY_H
#define PACKEDC_CRYPTO_CALLER_EXPORT_PUBLIC_KEY_H

#include <string.h>
#include <stdlib.h>
#include <psa/crypto.h>
#include <service/common/client/service_client.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <protocols/service/crypto/packed-c/export_public_key.h>
#include <common/tlv/tlv.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline psa_status_t crypto_caller_export_public_key(struct service_client *context,
	psa_key_id_t id,
	uint8_t *data, size_t data_size, size_t *data_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_export_public_key_in req_msg;
	size_t req_len = sizeof(struct ts_crypto_export_public_key_in);

	req_msg.id = id;

	*data_length = 0; /* For failure case */

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(context->caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;

		memcpy(req_buf, &req_msg, req_len);

		context->rpc_status =
			rpc_caller_invoke(context->caller, call_handle,
				TS_CRYPTO_OPCODE_EXPORT_PUBLIC_KEY, &opstatus, &resp_buf, &resp_len);

		if (context->rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				struct tlv_const_iterator resp_iter;
				struct tlv_record decoded_record;
				tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

				if (tlv_find_decode(&resp_iter,
						TS_CRYPTO_EXPORT_PUBLIC_KEY_OUT_TAG_DATA, &decoded_record)) {

					if (decoded_record.length <= data_size) {

						memcpy(data, decoded_record.value, decoded_record.length);
						*data_length = decoded_record.length;
					}
					else {
						/* Provided buffer is too small */
						psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
					}
				}
				else {
					/* Mandatory response parameter missing */
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

#endif /* PACKEDC_CRYPTO_CALLER_EXPORT_PUBLIC_KEY_H */
