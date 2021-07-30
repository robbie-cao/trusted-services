/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <psa/crypto.h>
#include "psa_crypto_client.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <protocols/service/crypto/packed-c/cipher.h>
#include <common/tlv/tlv.h>

static psa_status_t common_cipher_setup(psa_cipher_operation_t *operation,
	psa_key_id_t key,
	psa_algorithm_t alg,
	uint32_t opcode)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_cipher_setup_in req_msg;
	size_t req_len = sizeof(struct ts_crypto_cipher_setup_in);

	req_msg.key_id = key;
	req_msg.alg = alg;

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
				opcode, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.base.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				if (resp_len >= sizeof(struct ts_crypto_cipher_setup_out)) {

					struct ts_crypto_cipher_setup_out resp_msg;
					memcpy(&resp_msg, resp_buf, sizeof(struct ts_crypto_cipher_setup_out));
					operation->handle = resp_msg.op_handle;
				}
				else {
					/* Failed to decode response message */
					psa_status = PSA_ERROR_GENERIC_ERROR;
				}
			}
		}

		rpc_caller_end(psa_crypto_client_instance.base.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_cipher_encrypt_setup(psa_cipher_operation_t *operation,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	return common_cipher_setup(operation, key, alg, TS_CRYPTO_OPCODE_CIPHER_ENCRYPT_SETUP);
}

psa_status_t psa_cipher_decrypt_setup(psa_cipher_operation_t *operation,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	return common_cipher_setup(operation, key, alg, TS_CRYPTO_OPCODE_CIPHER_DECRYPT_SETUP);
}

psa_status_t psa_cipher_generate_iv(psa_cipher_operation_t *operation,
	uint8_t *iv,
	size_t iv_size,
	size_t *iv_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_cipher_generate_iv_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_cipher_generate_iv_in);
	size_t req_len = req_fixed_len;

	*iv_length = 0;
	req_msg.op_handle = operation->handle;

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.base.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;

		memcpy(req_buf, &req_msg, req_fixed_len);

		psa_crypto_client_instance.base.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.base.caller, call_handle,
				TS_CRYPTO_OPCODE_CIPHER_GENERATE_IV, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.base.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				struct tlv_const_iterator resp_iter;
				struct tlv_record decoded_record;
				tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

				if (tlv_find_decode(&resp_iter,
					TS_CRYPTO_CIPHER_GENERATE_IV_OUT_TAG_IV, &decoded_record)) {

					if (decoded_record.length <= iv_size) {

						memcpy(iv, decoded_record.value, decoded_record.length);
						*iv_length = decoded_record.length;
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

		rpc_caller_end(psa_crypto_client_instance.base.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_cipher_set_iv(psa_cipher_operation_t *operation,
	const uint8_t *iv,
	size_t iv_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_cipher_set_iv_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_cipher_set_iv_in);
	size_t req_len = req_fixed_len;

	req_msg.op_handle = operation->handle;

	/* Mandatory input data parameter */
	struct tlv_record data_record;
	data_record.tag = TS_CRYPTO_CIPHER_SET_IV_IN_TAG_IV;
	data_record.length = iv_length;
	data_record.value = iv;
	req_len += tlv_required_space(data_record.length);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.base.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;
		struct tlv_iterator req_iter;

		memcpy(req_buf, &req_msg, req_fixed_len);

		tlv_iterator_begin(&req_iter, &req_buf[req_fixed_len], req_len - req_fixed_len);
		tlv_encode(&req_iter, &data_record);

		psa_crypto_client_instance.base.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.base.caller, call_handle,
				TS_CRYPTO_OPCODE_CIPHER_SET_IV, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.base.rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

		rpc_caller_end(psa_crypto_client_instance.base.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_cipher_update(psa_cipher_operation_t *operation,
	const uint8_t *input,
	size_t input_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_cipher_update_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_cipher_update_in);
	size_t req_len = req_fixed_len;

	*output_length = 0;
	req_msg.op_handle = operation->handle;

	/* Mandatory input data parameter */
	struct tlv_record data_record;
	data_record.tag = TS_CRYPTO_CIPHER_UPDATE_IN_TAG_DATA;
	data_record.length = input_length;
	data_record.value = input;
	req_len += tlv_required_space(data_record.length);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.base.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;
		struct tlv_iterator req_iter;

		memcpy(req_buf, &req_msg, req_fixed_len);

		tlv_iterator_begin(&req_iter, &req_buf[req_fixed_len], req_len - req_fixed_len);
		tlv_encode(&req_iter, &data_record);

		psa_crypto_client_instance.base.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.base.caller, call_handle,
				TS_CRYPTO_OPCODE_CIPHER_UPDATE, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.base.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				struct tlv_const_iterator resp_iter;
				struct tlv_record decoded_record;
				tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

				if (tlv_find_decode(&resp_iter,
					TS_CRYPTO_CIPHER_UPDATE_OUT_TAG_DATA, &decoded_record)) {

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

		rpc_caller_end(psa_crypto_client_instance.base.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_cipher_finish(psa_cipher_operation_t *operation,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_cipher_finish_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_cipher_finish_in);
	size_t req_len = req_fixed_len;

	*output_length = 0;
	req_msg.op_handle = operation->handle;

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.base.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;

		memcpy(req_buf, &req_msg, req_fixed_len);

		psa_crypto_client_instance.base.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.base.caller, call_handle,
				TS_CRYPTO_OPCODE_CIPHER_FINISH, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.base.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				struct tlv_const_iterator resp_iter;
				struct tlv_record decoded_record;
				tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

				if (tlv_find_decode(&resp_iter,
					TS_CRYPTO_CIPHER_FINISH_OUT_TAG_DATA, &decoded_record)) {

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

		rpc_caller_end(psa_crypto_client_instance.base.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_cipher_abort(psa_cipher_operation_t *operation)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_cipher_abort_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_cipher_abort_in);
	size_t req_len = req_fixed_len;

	req_msg.op_handle = operation->handle;

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.base.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;

		memcpy(req_buf, &req_msg, req_fixed_len);

		psa_crypto_client_instance.base.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.base.caller, call_handle,
				TS_CRYPTO_OPCODE_CIPHER_ABORT, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.base.rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

		rpc_caller_end(psa_crypto_client_instance.base.caller, call_handle);
	}

	return psa_status;
}

static size_t max_cipher_update_size(void)
{
	/* Returns the maximum number of bytes that may be
	 * carried as a parameter of the cipher_update operation
	 * using the packed-c encoding.
	 */
	size_t payload_space = psa_crypto_client_instance.base.service_info.max_payload;
	size_t overhead = sizeof(struct ts_crypto_cipher_update_in) + TLV_HDR_LEN;

	return (payload_space > overhead) ? payload_space - overhead : 0;
}

static psa_status_t multi_cipher_update(psa_cipher_operation_t *operation,
	const uint8_t *input,
	size_t input_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	psa_status_t psa_status = PSA_SUCCESS;
	size_t max_update_size = max_cipher_update_size();
	size_t bytes_input = 0;
	size_t bytes_output = 0;

	*output_length = 0;

	if (!max_update_size) {

		/* Don't know the max update size so assume that the entire
		 * input and output can be handled in a single update.  If
		 * this isn't true, the first cipher update operation will fail
		 * safely.
		 */
		max_update_size = input_length;
	}

	while ((bytes_input < input_length) && (bytes_output < output_size)) {

		size_t update_output_len = 0;
		size_t bytes_remaining = input_length - bytes_input;
		size_t update_len = (bytes_remaining < max_update_size) ?
			bytes_remaining :
			max_update_size;

		psa_status = psa_cipher_update(operation,
			&input[bytes_input], update_len,
			&output[bytes_output], output_size - bytes_output, &update_output_len);

		if (psa_status != PSA_SUCCESS) {

			psa_cipher_abort(operation);
			break;
		}

		bytes_input += update_len;
		bytes_output += update_output_len;
	}

	if (psa_status == PSA_SUCCESS) {

		if (bytes_output < output_size) {

			size_t finish_output_len = 0;

			psa_status = psa_cipher_finish(operation,
				&output[bytes_output], output_size - bytes_output, &finish_output_len);

			if (psa_status == PSA_SUCCESS) {

				*output_length = bytes_output + finish_output_len;
			}
		}
		else {

			psa_cipher_abort(operation);
			psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
		}
	}

	return psa_status;
}

psa_status_t psa_cipher_encrypt(psa_key_id_t key,
	psa_algorithm_t alg,
	const uint8_t *input,
	size_t input_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	psa_cipher_operation_t operation = psa_cipher_operation_init();
	psa_status_t psa_status = psa_cipher_encrypt_setup(&operation, key, alg);

	if (psa_status == PSA_SUCCESS) {

		psa_status = multi_cipher_update(&operation,
			input, input_length,
			output, output_size, output_length);
	}

	return psa_status;
}

psa_status_t psa_cipher_decrypt(psa_key_id_t key,
	psa_algorithm_t alg,
	const uint8_t *input,
	size_t input_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	psa_cipher_operation_t operation = psa_cipher_operation_init();
	psa_status_t psa_status = psa_cipher_decrypt_setup(&operation, key, alg);

	if (psa_status == PSA_SUCCESS) {

		psa_status = multi_cipher_update(&operation,
			input, input_length,
			output, output_size, output_length);
	}

	return psa_status;
}
