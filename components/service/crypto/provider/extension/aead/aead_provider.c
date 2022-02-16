/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include <stdlib.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <service/crypto/provider/extension/aead/aead_provider.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <psa/crypto.h>

/* Service request handlers */
static rpc_status_t aead_setup_handler(void *context, struct call_req *req);
static rpc_status_t aead_generate_nonce_handler(void *context, struct call_req *req);
static rpc_status_t aead_set_nonce_handler(void *context, struct call_req *req);
static rpc_status_t aead_set_lengths_handler(void *context, struct call_req *req);
static rpc_status_t aead_update_ad_handler(void *context, struct call_req *req);
static rpc_status_t aead_update_handler(void *context, struct call_req *req);
static rpc_status_t aead_finish_handler(void *context, struct call_req *req);
static rpc_status_t aead_verify_handler(void *context, struct call_req *req);
static rpc_status_t aead_abort_handler(void *context, struct call_req *req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{TS_CRYPTO_OPCODE_AEAD_ENCRYPT_SETUP,		aead_setup_handler},
	{TS_CRYPTO_OPCODE_AEAD_DECRYPT_SETUP, 		aead_setup_handler},
	{TS_CRYPTO_OPCODE_AEAD_GENERATE_NONCE,		aead_generate_nonce_handler},
	{TS_CRYPTO_OPCODE_AEAD_SET_NONCE,			aead_set_nonce_handler},
	{TS_CRYPTO_OPCODE_AEAD_SET_LENGTHS,			aead_set_lengths_handler},
	{TS_CRYPTO_OPCODE_AEAD_UPDATE_AD,			aead_update_ad_handler},
	{TS_CRYPTO_OPCODE_AEAD_UPDATE,				aead_update_handler},
	{TS_CRYPTO_OPCODE_AEAD_FINISH,				aead_finish_handler},
	{TS_CRYPTO_OPCODE_AEAD_VERIFY,				aead_verify_handler},
	{TS_CRYPTO_OPCODE_AEAD_ABORT,				aead_abort_handler}
};

void aead_provider_init(struct aead_provider *context)
{
	crypto_context_pool_init(&context->context_pool);

	for (size_t encoding = 0; encoding < TS_RPC_ENCODING_LIMIT; ++encoding)
		context->serializers[encoding] = NULL;

	service_provider_init(&context->base_provider, context,
		handler_table, sizeof(handler_table)/sizeof(struct service_handler));
}

void aead_provider_deinit(struct aead_provider *context)
{
	crypto_context_pool_deinit(&context->context_pool);
}

void aead_provider_register_serializer(struct aead_provider *context,
	unsigned int encoding, const struct aead_provider_serializer *serializer)
{
	if (encoding < TS_RPC_ENCODING_LIMIT)
		context->serializers[encoding] = serializer;
}

static const struct aead_provider_serializer* get_serializer(void *context,
	const struct call_req *req)
{
	struct aead_provider *this_instance = (struct aead_provider*)context;
	const struct aead_provider_serializer* serializer = NULL;
	unsigned int encoding = call_req_get_encoding(req);

	if (encoding < TS_RPC_ENCODING_LIMIT) serializer = this_instance->serializers[encoding];

	return serializer;
}

static rpc_status_t aead_setup_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct aead_provider_serializer *serializer = get_serializer(context, req);
	struct aead_provider *this_instance = (struct aead_provider*)context;

	psa_key_id_t key_id;
	psa_algorithm_t alg;

	if (serializer)
		rpc_status = serializer->deserialize_aead_setup_req(req_buf, &key_id, &alg);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		uint32_t op_handle;

		struct crypto_context *crypto_context =
			crypto_context_pool_alloc(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_AEAD, call_req_get_caller_id(req),
				&op_handle);

		if (crypto_context) {

			psa_status_t psa_status;

			crypto_context->op.aead = psa_aead_operation_init();

			psa_status = (call_req_get_opcode(req) == TS_CRYPTO_OPCODE_AEAD_ENCRYPT_SETUP) ?
				psa_aead_encrypt_setup(&crypto_context->op.aead, key_id, alg) :
				psa_aead_decrypt_setup(&crypto_context->op.aead, key_id, alg);

			if (psa_status == PSA_SUCCESS) {

				struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
				rpc_status = serializer->serialize_aead_setup_resp(resp_buf, op_handle);
			}

			if ((psa_status != PSA_SUCCESS) || (rpc_status != TS_RPC_CALL_ACCEPTED)) {

				crypto_context_pool_free(&this_instance->context_pool, crypto_context);
			}

			call_req_set_opstatus(req, psa_status);
		}
		else {
			/* Failed to allocate crypto context for transaction */
			rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
		}
	}

	return rpc_status;
}

static rpc_status_t aead_generate_nonce_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct aead_provider_serializer *serializer = get_serializer(context, req);
	struct aead_provider *this_instance = (struct aead_provider*)context;

	uint32_t op_handle;

	if (serializer)
		rpc_status = serializer->deserialize_aead_generate_nonce_req(req_buf, &op_handle);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_AEAD, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			size_t nonce_len;
			uint8_t nonce[PSA_AEAD_NONCE_MAX_SIZE];

			psa_status = psa_aead_generate_nonce(&crypto_context->op.aead,
				nonce, sizeof(nonce), &nonce_len);

			if (psa_status == PSA_SUCCESS) {

				struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
				rpc_status = serializer->serialize_aead_generate_nonce_resp(resp_buf,
					nonce, nonce_len);
			}
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t aead_set_nonce_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct aead_provider_serializer *serializer = get_serializer(context, req);
	struct aead_provider *this_instance = (struct aead_provider*)context;

	uint32_t op_handle;
	const uint8_t *nonce;
	size_t nonce_len;

	if (serializer)
		rpc_status = serializer->deserialize_aead_set_nonce_req(req_buf, &op_handle,
			&nonce, &nonce_len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_AEAD, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			psa_status = psa_aead_set_nonce(&crypto_context->op.aead, nonce, nonce_len);
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t aead_set_lengths_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct aead_provider_serializer *serializer = get_serializer(context, req);
	struct aead_provider *this_instance = (struct aead_provider*)context;

	uint32_t op_handle;
	size_t ad_length;
	size_t plaintext_length;

	if (serializer)
		rpc_status = serializer->deserialize_aead_set_lengths_req(req_buf, &op_handle,
			&ad_length, &plaintext_length);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_AEAD, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			psa_status = psa_aead_set_lengths(&crypto_context->op.aead,
				ad_length, plaintext_length);
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t aead_update_ad_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct aead_provider_serializer *serializer = get_serializer(context, req);
	struct aead_provider *this_instance = (struct aead_provider*)context;

	uint32_t op_handle;
	const uint8_t *input;
	size_t input_len;

	if (serializer)
		rpc_status = serializer->deserialize_aead_update_ad_req(req_buf, &op_handle,
			&input, &input_len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_AEAD, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			psa_status = psa_aead_update_ad(&crypto_context->op.aead, input, input_len);
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t aead_update_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct aead_provider_serializer *serializer = get_serializer(context, req);
	struct aead_provider *this_instance = (struct aead_provider*)context;

	uint32_t op_handle;
	const uint8_t *input;
	size_t input_len;

	if (serializer)
		rpc_status = serializer->deserialize_aead_update_req(req_buf, &op_handle,
			&input, &input_len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_AEAD, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			size_t output_len = 0;
			size_t output_size = PSA_AEAD_UPDATE_OUTPUT_MAX_SIZE(input_len);
			uint8_t *output = malloc(output_size);

			if (output) {

				psa_status = psa_aead_update(&crypto_context->op.aead,
					input, input_len,
					output, output_size, &output_len);

				if (psa_status == PSA_SUCCESS) {

					struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
					rpc_status = serializer->serialize_aead_update_resp(resp_buf,
						output, output_len);
				}

				free(output);
			}
			else {

				psa_status = PSA_ERROR_INSUFFICIENT_MEMORY;
			}
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t aead_finish_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct aead_provider_serializer *serializer = get_serializer(context, req);
	struct aead_provider *this_instance = (struct aead_provider*)context;

	uint32_t op_handle;

	if (serializer)
		rpc_status = serializer->deserialize_aead_finish_req(req_buf, &op_handle);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_AEAD, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			size_t ciphertext_len;
			uint8_t ciphertext[PSA_AEAD_FINISH_OUTPUT_MAX_SIZE];

			size_t tag_len;
			uint8_t tag[PSA_AEAD_TAG_MAX_SIZE];

			psa_status = psa_aead_finish(&crypto_context->op.aead,
				ciphertext, sizeof(ciphertext), &ciphertext_len,
				tag, sizeof(tag), &tag_len);

			if (psa_status == PSA_SUCCESS) {

				struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
				rpc_status = serializer->serialize_aead_finish_resp(resp_buf,
					ciphertext, ciphertext_len,
					tag, tag_len);

				crypto_context_pool_free(&this_instance->context_pool, crypto_context);
			}
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t aead_verify_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct aead_provider_serializer *serializer = get_serializer(context, req);
	struct aead_provider *this_instance = (struct aead_provider*)context;

	uint32_t op_handle;
	const uint8_t *tag;
	size_t tag_len;

	if (serializer)
		rpc_status = serializer->deserialize_aead_verify_req(req_buf, &op_handle,
			&tag, &tag_len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_AEAD, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			size_t plaintext_len;
			uint8_t plaintext[PSA_AEAD_VERIFY_OUTPUT_MAX_SIZE];

			psa_status = psa_aead_verify(&crypto_context->op.aead,
				plaintext, sizeof(plaintext), &plaintext_len,
				tag, tag_len);

			if (psa_status == PSA_SUCCESS) {

				struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
				rpc_status = serializer->serialize_aead_verify_resp(resp_buf,
					plaintext, plaintext_len);

				crypto_context_pool_free(&this_instance->context_pool, crypto_context);
			}
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t aead_abort_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct aead_provider_serializer *serializer = get_serializer(context, req);
	struct aead_provider *this_instance = (struct aead_provider*)context;

	uint32_t op_handle;

	if (serializer)
		rpc_status = serializer->deserialize_aead_abort_req(req_buf, &op_handle);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		/* Return success if operation is no longer active and
		 * doesn't need aborting.
		 */
		psa_status_t psa_status = PSA_SUCCESS;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_AEAD, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			psa_status = psa_aead_abort(&crypto_context->op.aead);
			crypto_context_pool_free(&this_instance->context_pool, crypto_context);
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}
