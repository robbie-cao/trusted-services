/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include <stdlib.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <service/crypto/provider/extension/cipher/cipher_provider.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <psa/crypto.h>

/* Service request handlers */
static rpc_status_t cipher_setup_handler(void *context, struct call_req* req);
static rpc_status_t cipher_generate_iv_handler(void *context, struct call_req* req);
static rpc_status_t cipher_set_iv_handler(void *context, struct call_req* req);
static rpc_status_t cipher_update_handler(void *context, struct call_req* req);
static rpc_status_t cipher_finish_handler(void *context, struct call_req* req);
static rpc_status_t cipher_abort_handler(void *context, struct call_req* req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{TS_CRYPTO_OPCODE_CIPHER_ENCRYPT_SETUP,   	cipher_setup_handler},
	{TS_CRYPTO_OPCODE_CIPHER_DECRYPT_SETUP,   	cipher_setup_handler},
	{TS_CRYPTO_OPCODE_CIPHER_GENERATE_IV,   	cipher_generate_iv_handler},
	{TS_CRYPTO_OPCODE_CIPHER_SET_IV,   			cipher_set_iv_handler},
	{TS_CRYPTO_OPCODE_CIPHER_UPDATE,          	cipher_update_handler},
	{TS_CRYPTO_OPCODE_CIPHER_FINISH,          	cipher_finish_handler},
	{TS_CRYPTO_OPCODE_CIPHER_ABORT,          	cipher_abort_handler}
};

void cipher_provider_init(struct cipher_provider *context)
{
	crypto_context_pool_init(&context->context_pool);

	for (size_t encoding = 0; encoding < TS_RPC_ENCODING_LIMIT; ++encoding)
		context->serializers[encoding] = NULL;

	service_provider_init(&context->base_provider, context,
		handler_table, sizeof(handler_table)/sizeof(struct service_handler));
}

void cipher_provider_deinit(struct cipher_provider *context)
{
	crypto_context_pool_deinit(&context->context_pool);
}

void cipher_provider_register_serializer(struct cipher_provider *context,
	unsigned int encoding, const struct cipher_provider_serializer *serializer)
{
	if (encoding < TS_RPC_ENCODING_LIMIT)
		context->serializers[encoding] = serializer;
}

static const struct cipher_provider_serializer* get_serializer(void *context,
	const struct call_req *req)
{
	struct cipher_provider *this_instance = (struct cipher_provider*)context;
	const struct cipher_provider_serializer* serializer = NULL;
	unsigned int encoding = call_req_get_encoding(req);

	if (encoding < TS_RPC_ENCODING_LIMIT) serializer = this_instance->serializers[encoding];

	return serializer;
}

static rpc_status_t cipher_setup_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct cipher_provider_serializer *serializer = get_serializer(context, req);
	struct cipher_provider *this_instance = (struct cipher_provider*)context;

	psa_key_id_t key_id;
	psa_algorithm_t alg;

	if (serializer)
		rpc_status = serializer->deserialize_cipher_setup_req(req_buf, &key_id, &alg);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		uint32_t op_handle;

		struct crypto_context *crypto_context =
			crypto_context_pool_alloc(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_CIPHER, call_req_get_caller_id(req),
				&op_handle);

		if (crypto_context) {

			psa_status_t psa_status;

			crypto_context->op.cipher = psa_cipher_operation_init();

			psa_status = (call_req_get_opcode(req) == TS_CRYPTO_OPCODE_CIPHER_ENCRYPT_SETUP) ?
				psa_cipher_encrypt_setup(&crypto_context->op.cipher, key_id, alg) :
				psa_cipher_decrypt_setup(&crypto_context->op.cipher, key_id, alg);

			if (psa_status == PSA_SUCCESS) {

				struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
				rpc_status = serializer->serialize_cipher_setup_resp(resp_buf, op_handle);
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

static rpc_status_t cipher_generate_iv_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct cipher_provider_serializer *serializer = get_serializer(context, req);
	struct cipher_provider *this_instance = (struct cipher_provider*)context;

	uint32_t op_handle;

	if (serializer)
		rpc_status = serializer->deserialize_cipher_generate_iv_req(req_buf, &op_handle);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_CIPHER, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			size_t iv_len;
			uint8_t iv[PSA_CIPHER_IV_MAX_SIZE];

			psa_status = psa_cipher_generate_iv(&crypto_context->op.cipher, iv, sizeof(iv), &iv_len);

			if (psa_status == PSA_SUCCESS) {

				struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
				rpc_status = serializer->serialize_cipher_generate_iv_resp(resp_buf, iv, iv_len);
			}
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t cipher_set_iv_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct cipher_provider_serializer *serializer = get_serializer(context, req);
	struct cipher_provider *this_instance = (struct cipher_provider*)context;

	uint32_t op_handle;
	const uint8_t *iv;
	size_t iv_len;

	if (serializer)
		rpc_status = serializer->deserialize_cipher_set_iv_req(req_buf, &op_handle,
			&iv, &iv_len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_CIPHER, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			psa_status = psa_cipher_set_iv(&crypto_context->op.cipher, iv, iv_len);
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t cipher_update_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct cipher_provider_serializer *serializer = get_serializer(context, req);
	struct cipher_provider *this_instance = (struct cipher_provider*)context;

	uint32_t op_handle;
	const uint8_t *input;
	size_t input_len;

	if (serializer)
		rpc_status = serializer->deserialize_cipher_update_req(req_buf, &op_handle,
			&input, &input_len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_CIPHER, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			size_t output_len = 0;
			size_t output_size = PSA_CIPHER_UPDATE_OUTPUT_MAX_SIZE(input_len);
			uint8_t *output = malloc(output_size);

			if (output) {

				psa_status = psa_cipher_update(&crypto_context->op.cipher,
					input, input_len,
					output, output_size, &output_len);

				if (psa_status == PSA_SUCCESS) {

					struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
					rpc_status = serializer->serialize_cipher_update_resp(resp_buf,
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

static rpc_status_t cipher_finish_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct cipher_provider_serializer *serializer = get_serializer(context, req);
	struct cipher_provider *this_instance = (struct cipher_provider*)context;

	uint32_t op_handle;

	if (serializer)
		rpc_status = serializer->deserialize_cipher_finish_req(req_buf, &op_handle);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_CIPHER, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			size_t output_len;
			uint8_t output[PSA_CIPHER_FINISH_OUTPUT_MAX_SIZE];

			psa_status = psa_cipher_finish(&crypto_context->op.cipher, output, sizeof(output), &output_len);

			if (psa_status == PSA_SUCCESS) {

				struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
				rpc_status = serializer->serialize_cipher_finish_resp(resp_buf, output, output_len);

				crypto_context_pool_free(&this_instance->context_pool, crypto_context);
			}
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t cipher_abort_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct cipher_provider_serializer *serializer = get_serializer(context, req);
	struct cipher_provider *this_instance = (struct cipher_provider*)context;

	uint32_t op_handle;

	if (serializer)
		rpc_status = serializer->deserialize_cipher_abort_req(req_buf, &op_handle);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		/* Return success if operation is no longer active and
		 * doesn't need aborting.
		 */
		psa_status_t psa_status = PSA_SUCCESS;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_CIPHER, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			psa_status = psa_cipher_abort(&crypto_context->op.cipher);
			crypto_context_pool_free(&this_instance->context_pool, crypto_context);
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}
