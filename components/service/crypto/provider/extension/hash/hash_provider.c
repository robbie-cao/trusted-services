/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include <stdlib.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <service/crypto/provider/extension/hash/hash_provider.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <psa/crypto.h>

/* Service request handlers */
static rpc_status_t hash_setup_handler(void *context, struct call_req* req);
static rpc_status_t hash_update_handler(void *context, struct call_req* req);
static rpc_status_t hash_finish_handler(void *context, struct call_req* req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{TS_CRYPTO_OPCODE_HASH_SETUP,           hash_setup_handler},
	{TS_CRYPTO_OPCODE_HASH_UPDATE,          hash_update_handler},
	{TS_CRYPTO_OPCODE_HASH_FINISH,          hash_finish_handler}
};

void hash_provider_init(struct hash_provider *context)
{
	crypto_context_pool_init(&context->context_pool);

	for (size_t encoding = 0; encoding < TS_RPC_ENCODING_LIMIT; ++encoding)
		context->serializers[encoding] = NULL;

	service_provider_init(&context->base_provider, context,
		handler_table, sizeof(handler_table)/sizeof(struct service_handler));
}

void hash_provider_deinit(struct hash_provider *context)
{
	crypto_context_pool_deinit(&context->context_pool);
}

void hash_provider_register_serializer(struct hash_provider *context,
	unsigned int encoding, const struct hash_provider_serializer *serializer)
{
	if (encoding < TS_RPC_ENCODING_LIMIT)
		context->serializers[encoding] = serializer;
}

static const struct hash_provider_serializer* get_serializer(void *context,
	const struct call_req *req)
{
	struct hash_provider *this_instance = (struct hash_provider*)context;
	const struct hash_provider_serializer* serializer = NULL;
	unsigned int encoding = call_req_get_encoding(req);

	if (encoding < TS_RPC_ENCODING_LIMIT) serializer = this_instance->serializers[encoding];

	return serializer;
}

static rpc_status_t hash_setup_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct hash_provider_serializer *serializer = get_serializer(context, req);
	struct hash_provider *this_instance = (struct hash_provider*)context;

	psa_algorithm_t alg;

	if (serializer)
		rpc_status = serializer->deserialize_hash_setup_req(req_buf, &alg);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		uint32_t op_handle;

		struct crypto_context *crypto_context =
			crypto_context_pool_alloc(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_HASH, call_req_get_caller_id(req),
				&op_handle);

		if (crypto_context) {

			psa_status_t psa_status;

			crypto_context->op.hash = psa_hash_operation_init();
			psa_status = psa_hash_setup(&crypto_context->op.hash, alg);

			if (psa_status == PSA_SUCCESS) {

				struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
				rpc_status = serializer->serialize_hash_setup_resp(resp_buf, op_handle);
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

static rpc_status_t hash_update_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct hash_provider_serializer *serializer = get_serializer(context, req);
	struct hash_provider *this_instance = (struct hash_provider*)context;

	uint32_t op_handle;
	const uint8_t *data;
	size_t data_len;

	if (serializer)
		rpc_status = serializer->deserialize_hash_update_req(req_buf, &op_handle, &data, &data_len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_HASH, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			psa_status_t psa_status = psa_hash_update(&crypto_context->op.hash, data, data_len);
			call_req_set_opstatus(req, psa_status);
		}
		else {
			/* Requested context doesn't exist */
			rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
		}
	}

	return rpc_status;
}

static rpc_status_t hash_finish_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct hash_provider_serializer *serializer = get_serializer(context, req);
	struct hash_provider *this_instance = (struct hash_provider*)context;

	uint32_t op_handle;

	if (serializer)
		rpc_status = serializer->deserialize_hash_finish_req(req_buf, &op_handle);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_HASH, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			psa_status_t psa_status;
			size_t hash_len;
			uint8_t hash[PSA_HASH_MAX_SIZE];

			psa_status = psa_hash_finish(&crypto_context->op.hash, hash, sizeof(hash), &hash_len);

			if (psa_status == PSA_SUCCESS) {

				struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
				rpc_status = serializer->serialize_hash_finish_resp(resp_buf, hash, hash_len);
			}

			crypto_context_pool_free(&this_instance->context_pool, crypto_context);

			call_req_set_opstatus(req, psa_status);
		}
		else {
			/* Requested context doesn't exist */
			rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
		}
	}

	return rpc_status;
}
