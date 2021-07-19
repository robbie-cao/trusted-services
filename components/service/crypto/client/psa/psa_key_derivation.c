/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <psa/crypto.h>
#include "psa_crypto_client.h"
#include "psa_crypto_client_key_attributes.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <protocols/service/crypto/packed-c/key_derivation.h>
#include <common/tlv/tlv.h>

psa_status_t psa_key_derivation_setup(
	psa_key_derivation_operation_t *operation,
	psa_algorithm_t alg)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_key_derivation_setup_in req_msg;
	size_t req_len = sizeof(struct ts_crypto_key_derivation_setup_in);

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
				TS_CRYPTO_OPCODE_KEY_DERIVATION_SETUP, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				if (resp_len >= sizeof(struct ts_crypto_key_derivation_setup_out)) {

					struct ts_crypto_key_derivation_setup_out resp_msg;
					memcpy(&resp_msg, resp_buf, sizeof(struct ts_crypto_key_derivation_setup_out));
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

psa_status_t psa_key_derivation_get_capacity(
	const psa_key_derivation_operation_t *operation,
	size_t *capacity)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_key_derivation_get_capacity_in req_msg;
	size_t req_len = sizeof(struct ts_crypto_key_derivation_get_capacity_in);

	req_msg.op_handle = operation->handle;

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
				TS_CRYPTO_OPCODE_KEY_DERIVATION_GET_CAPACITY, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				if (resp_len >= sizeof(struct ts_crypto_key_derivation_get_capacity_out)) {

					struct ts_crypto_key_derivation_get_capacity_out resp_msg;
					memcpy(&resp_msg, resp_buf, sizeof(struct ts_crypto_key_derivation_get_capacity_out));
					*capacity = resp_msg.capacity;
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

psa_status_t psa_key_derivation_set_capacity(
	psa_key_derivation_operation_t *operation,
	size_t capacity)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_key_derivation_set_capacity_in req_msg;
	size_t req_len = sizeof(struct ts_crypto_key_derivation_set_capacity_in);

	req_msg.op_handle = operation->handle;
	req_msg.capacity = capacity;

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
				TS_CRYPTO_OPCODE_KEY_DERIVATION_SET_CAPACITY, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

		rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_key_derivation_input_bytes(
	psa_key_derivation_operation_t *operation,
	psa_key_derivation_step_t step,
	const uint8_t *data,
	size_t data_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_key_derivation_input_bytes_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_key_derivation_input_bytes_in);
	size_t req_len = req_fixed_len;

	req_msg.op_handle = operation->handle;
	req_msg.step = step;

	/* Mandatory input data parameter */
	struct tlv_record data_record;
	data_record.tag = TS_CRYPTO_KEY_DERIVATION_INPUT_BYTES_IN_TAG_DATA;
	data_record.length = data_length;
	data_record.value = data;
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
				TS_CRYPTO_OPCODE_KEY_DERIVATION_INPUT_BYTES, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

		rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_key_derivation_input_key(
	psa_key_derivation_operation_t *operation,
	psa_key_derivation_step_t step,
	psa_key_id_t key)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_key_derivation_input_key_in req_msg;
	size_t req_len = sizeof(struct ts_crypto_key_derivation_input_key_in);

	req_msg.op_handle = operation->handle;
	req_msg.step = step;
	req_msg.key_id = key;

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
				TS_CRYPTO_OPCODE_KEY_DERIVATION_INPUT_KEY, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

		rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_key_derivation_output_bytes(
	psa_key_derivation_operation_t *operation,
	uint8_t *output,
	size_t output_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_key_derivation_output_bytes_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_key_derivation_output_bytes_in);
	size_t req_len = req_fixed_len;

	req_msg.op_handle = operation->handle;
	req_msg.output_len = output_length;

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
				TS_CRYPTO_OPCODE_KEY_DERIVATION_OUTPUT_BYTES, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				struct tlv_const_iterator resp_iter;
				struct tlv_record decoded_record;
				tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

				if (tlv_find_decode(&resp_iter,
					TS_CRYPTO_KEY_DERIVATION_OUTPUT_BYTES_OUT_TAG_DATA, &decoded_record)) {

					if (decoded_record.length == output_length) {

						memcpy(output, decoded_record.value, decoded_record.length);
					}
					else {
						/* Should have returned the requested number of bytes */
						psa_status = PSA_ERROR_GENERIC_ERROR;
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

psa_status_t psa_key_derivation_output_key(
	const psa_key_attributes_t *attributes,
	psa_key_derivation_operation_t *operation,
	psa_key_id_t *key)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_key_derivation_output_key_in req_msg;
	size_t req_len = sizeof(struct ts_crypto_key_derivation_output_key_in);

	/* Set default outputs for failure case */
	*key = 0;

	req_msg.op_handle = operation->handle;
	psa_crypto_client_translate_key_attributes_to_proto(&req_msg.attributes, attributes);

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
				TS_CRYPTO_OPCODE_KEY_DERIVATION_OUTPUT_KEY, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				if (resp_len >= sizeof(struct ts_crypto_key_derivation_output_key_out)) {

					struct ts_crypto_key_derivation_output_key_out resp_msg;
					memcpy(&resp_msg, resp_buf,
						sizeof(struct ts_crypto_key_derivation_output_key_out));
					*key = resp_msg.key_id;
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

psa_status_t psa_key_derivation_abort(
	psa_key_derivation_operation_t *operation)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_key_derivation_abort_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_key_derivation_abort_in);
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
				TS_CRYPTO_OPCODE_KEY_DERIVATION_ABORT, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

		rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_key_derivation_key_agreement(
	psa_key_derivation_operation_t *operation,
	psa_key_derivation_step_t step,
	psa_key_id_t private_key,
	const uint8_t *peer_key,
	size_t peer_key_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_key_derivation_key_agreement_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_key_derivation_key_agreement_in);
	size_t req_len = req_fixed_len;

	req_msg.op_handle = operation->handle;
	req_msg.step = step;
	req_msg.private_key_id = private_key;

	/* Mandatory input data parameter */
	struct tlv_record data_record;
	data_record.tag = TS_CRYPTO_KEY_DERIVATION_KEY_AGREEMENT_IN_TAG_PEER_KEY;
	data_record.length = peer_key_length;
	data_record.value = peer_key;
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
				TS_CRYPTO_OPCODE_KEY_DERIVATION_KEY_AGREEMENT, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

		rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
	}

	return psa_status;
}

psa_status_t psa_raw_key_agreement(psa_algorithm_t alg,
	psa_key_id_t private_key,
	const uint8_t *peer_key,
	size_t peer_key_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	struct ts_crypto_raw_key_agreement_in req_msg;
	size_t req_fixed_len = sizeof(struct ts_crypto_raw_key_agreement_in);
	size_t req_len = req_fixed_len;

	req_msg.alg = alg;
	req_msg.private_key_id = private_key;

	/* Mandatory input data parameter */
	struct tlv_record data_record;
	data_record.tag = TS_CRYPTO_RAW_KEY_AGREEMENT_IN_TAG_PEER_KEY;
	data_record.length = peer_key_length;
	data_record.value = peer_key;
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
				TS_CRYPTO_OPCODE_KEY_DERIVATION_RAW_KEY_AGREEMENT, &opstatus, &resp_buf, &resp_len);

		if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				struct tlv_const_iterator resp_iter;
				struct tlv_record decoded_record;
				tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

				if (tlv_find_decode(&resp_iter,
					TS_CRYPTO_RAW_KEY_AGREEMENT_OUT_TAG_OUTPUT, &decoded_record)) {

					if (decoded_record.length <= output_size) {

						memcpy(output, decoded_record.value, decoded_record.length);
						*output_length = decoded_record.length;
					}
					else {
						/* Insufficient buffer space */
						psa_status = PSA_ERROR_INVALID_ARGUMENT;
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
