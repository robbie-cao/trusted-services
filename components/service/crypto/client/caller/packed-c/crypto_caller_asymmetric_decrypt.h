/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PACKEDC_CRYPTO_CALLER_ASYMMETRIC_DECRYPT_H
#define PACKEDC_CRYPTO_CALLER_ASYMMETRIC_DECRYPT_H

#include <string.h>
#include <stdlib.h>
#include <psa/crypto.h>
#include <service/common/client/service_client.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <protocols/service/crypto/packed-c/asymmetric_decrypt.h>
#include <common/tlv/tlv.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline psa_status_t crypto_caller_asymmetric_decrypt(struct service_client *context,
	psa_key_id_t id,
	psa_algorithm_t alg,
	const uint8_t *input, size_t input_length,
	const uint8_t *salt, size_t salt_length,
	uint8_t *output, size_t output_size, size_t *output_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_asymmetric_decrypt_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_asymmetric_decrypt_in);
	size_t req_len = req_fixed_len;

	*output_length = 0;  /* For failure case */

	req_msg.id = id;
	req_msg.alg = alg;

	/* Mandatory parameter */
	struct tlv_record ciphertext_record;
	ciphertext_record.tag = TS_CRYPTO_ASYMMETRIC_DECRYPT_IN_TAG_CIPHERTEXT;
	ciphertext_record.length = input_length;
	ciphertext_record.value = input;
	req_len += tlv_required_space(ciphertext_record.length);

	/* Optional parameter */
	struct tlv_record salt_record;
	salt_record.tag = TS_CRYPTO_ASYMMETRIC_DECRYPT_IN_TAG_SALT;
	salt_record.length = (salt) ? salt_length : 0;
	salt_record.value = salt;
	if (salt) req_len += tlv_required_space(salt_record.length);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(context->caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		rpc_opstatus_t opstatus;
		struct tlv_iterator req_iter;

		memcpy(req_buf, &req_msg, req_fixed_len);

		tlv_iterator_begin(&req_iter, &req_buf[req_fixed_len], req_len - req_fixed_len);
		tlv_encode(&req_iter, &ciphertext_record);
		if (salt) tlv_encode(&req_iter, &salt_record);

		context->rpc_status =
			rpc_caller_invoke(context->caller, call_handle,
					TS_CRYPTO_OPCODE_ASYMMETRIC_DECRYPT, &opstatus, &resp_buf, &resp_len);

		if (context->rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				struct tlv_const_iterator resp_iter;
				struct tlv_record decoded_record;
				tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

				if (tlv_find_decode(&resp_iter,
					TS_CRYPTO_ASYMMETRIC_DECRYPT_OUT_TAG_PLAINTEXT, &decoded_record)) {

					if (decoded_record.length <= output_size) {

						memcpy(output, decoded_record.value, decoded_record.length);
						*output_length = decoded_record.length;
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

#endif /* PACKEDC_CRYPTO_CALLER_ASYMMETRIC_DECRYPT_H */
