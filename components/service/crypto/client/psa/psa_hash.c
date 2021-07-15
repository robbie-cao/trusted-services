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
#include <protocols/service/crypto/packed-c/hash.h>
#include <common/tlv/tlv.h>

psa_status_t psa_hash_setup(psa_hash_operation_t *operation,
	psa_algorithm_t alg)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_hash_setup_in req_msg;
	size_t req_len = sizeof(struct ts_crypto_hash_setup_in);

	req_msg.alg = alg;

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
				TS_CRYPTO_OPCODE_HASH_SETUP, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				if (resp_len >= sizeof(struct ts_crypto_hash_setup_out)) {

					struct ts_crypto_hash_setup_out resp_msg;
					memcpy(&resp_msg, resp_buf, sizeof(struct ts_crypto_hash_setup_out));
					operation->handle = resp_msg.op_handle;
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

psa_status_t psa_hash_update(psa_hash_operation_t *operation,
	const uint8_t *input,
	size_t input_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_hash_update_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_hash_update_in);
	size_t req_len = req_fixed_len;

	req_msg.op_handle = operation->handle;

	/* Mandatory input data parameter */
	struct tlv_record data_record;
	data_record.tag = TS_CRYPTO_HASH_UPDATE_IN_TAG_DATA;
	data_record.length = input_length;
	data_record.value = input;
	req_len += tlv_required_space(data_record.length);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;
		struct tlv_iterator req_iter;

		memcpy(req_buf, &req_msg, req_fixed_len);

		tlv_iterator_begin(&req_iter, &req_buf[req_fixed_len], req_len - req_fixed_len);
		tlv_encode(&req_iter, &data_record);

		psa_crypto_client_instance.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.caller, call_handle,
				TS_CRYPTO_OPCODE_HASH_UPDATE, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

		rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_hash_finish(psa_hash_operation_t *operation,
	uint8_t *hash,
	size_t hash_size,
	size_t *hash_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_hash_finish_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_hash_finish_in);
	size_t req_len = req_fixed_len;

	*hash_length = 0;
	req_msg.op_handle = operation->handle;

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;

		memcpy(req_buf, &req_msg, req_fixed_len);

		psa_crypto_client_instance.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.caller, call_handle,
				TS_CRYPTO_OPCODE_HASH_FINISH, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				struct tlv_const_iterator resp_iter;
				struct tlv_record decoded_record;
				tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

				if (tlv_find_decode(&resp_iter,
					TS_CRYPTO_HASH_FINISH_OUT_TAG_HASH, &decoded_record)) {

					if (decoded_record.length <= hash_size) {

						memcpy(hash, decoded_record.value, decoded_record.length);
						*hash_length = decoded_record.length;
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

		rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_hash_abort(psa_hash_operation_t *operation)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_hash_abort_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_hash_abort_in);
	size_t req_len = req_fixed_len;

	req_msg.op_handle = operation->handle;

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;

		memcpy(req_buf, &req_msg, req_fixed_len);

		psa_crypto_client_instance.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.caller, call_handle,
				TS_CRYPTO_OPCODE_HASH_ABORT, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

		rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_hash_verify(psa_hash_operation_t *operation,
	const uint8_t *hash,
	size_t hash_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_hash_verify_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_hash_verify_in);
	size_t req_len = req_fixed_len;

	req_msg.op_handle = operation->handle;

	/* Mandatory input data parameter */
	struct tlv_record data_record;
	data_record.tag = TS_CRYPTO_HASH_VERIFY_IN_TAG_HASH;
	data_record.length = hash_length;
	data_record.value = hash;
	req_len += tlv_required_space(data_record.length);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;
		struct tlv_iterator req_iter;

		memcpy(req_buf, &req_msg, req_fixed_len);

		tlv_iterator_begin(&req_iter, &req_buf[req_fixed_len], req_len - req_fixed_len);
		tlv_encode(&req_iter, &data_record);

		psa_crypto_client_instance.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.caller, call_handle,
				TS_CRYPTO_OPCODE_HASH_VERIFY, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

		rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_hash_clone(const psa_hash_operation_t *source_operation,
	psa_hash_operation_t *target_operation)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_hash_clone_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_hash_clone_in);
	size_t req_len = req_fixed_len;

	req_msg.source_op_handle = source_operation->handle;

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(psa_crypto_client_instance.caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;

		memcpy(req_buf, &req_msg, req_fixed_len);

		psa_crypto_client_instance.rpc_status =
			rpc_caller_invoke(psa_crypto_client_instance.caller, call_handle,
				TS_CRYPTO_OPCODE_HASH_CLONE, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				if (resp_len >= sizeof(struct ts_crypto_hash_clone_out)) {

					struct ts_crypto_hash_clone_out resp_msg;
					memcpy(&resp_msg, resp_buf, sizeof(struct ts_crypto_hash_clone_out));
					target_operation->handle = resp_msg.target_op_handle;
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

psa_status_t psa_hash_compare(psa_algorithm_t alg,
	const uint8_t *input,
	size_t input_length,
	const uint8_t *hash,
	size_t hash_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_hash_compute(psa_algorithm_t alg,
	const uint8_t *input,
	size_t input_length,
	uint8_t *hash,
	size_t hash_size,
	size_t *hash_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}
