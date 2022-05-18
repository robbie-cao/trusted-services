/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include <stdlib.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <service/crypto/provider/extension/mac/mac_provider.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <psa/crypto.h>

/* Service request handlers */
static rpc_status_t mac_setup_handler(void *context, struct call_req* req);
static rpc_status_t mac_update_handler(void *context, struct call_req* req);
static rpc_status_t mac_sign_finish_handler(void *context, struct call_req* req);
static rpc_status_t mac_verify_finish_handler(void *context, struct call_req* req);
static rpc_status_t mac_abort_handler(void *context, struct call_req* req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{TS_CRYPTO_OPCODE_MAC_SIGN_SETUP,       mac_setup_handler},
	{TS_CRYPTO_OPCODE_MAC_VERIFY_SETUP,     mac_setup_handler},
	{TS_CRYPTO_OPCODE_MAC_UPDATE,          	mac_update_handler},
	{TS_CRYPTO_OPCODE_MAC_SIGN_FINISH,      mac_sign_finish_handler},
	{TS_CRYPTO_OPCODE_MAC_VERIFY_FINISH,    mac_verify_finish_handler},
	{TS_CRYPTO_OPCODE_MAC_ABORT,          	mac_abort_handler}
};

void mac_provider_init(struct mac_provider *context)
{
	crypto_context_pool_init(&context->context_pool);

	for (size_t encoding = 0; encoding < TS_RPC_ENCODING_LIMIT; ++encoding)
		context->serializers[encoding] = NULL;

	service_provider_init(&context->base_provider, context,
		handler_table, sizeof(handler_table)/sizeof(struct service_handler));
}

void mac_provider_deinit(struct mac_provider *context)
{
	crypto_context_pool_deinit(&context->context_pool);
}

void mac_provider_register_serializer(struct mac_provider *context,
	unsigned int encoding, const struct mac_provider_serializer *serializer)
{
	if (encoding < TS_RPC_ENCODING_LIMIT)
		context->serializers[encoding] = serializer;
}

static const struct mac_provider_serializer* get_serializer(void *context,
	const struct call_req *req)
{
	struct mac_provider *this_instance = (struct mac_provider*)context;
	const struct mac_provider_serializer* serializer = NULL;
	unsigned int encoding = call_req_get_encoding(req);

	if (encoding < TS_RPC_ENCODING_LIMIT) serializer = this_instance->serializers[encoding];

	return serializer;
}

static rpc_status_t mac_setup_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct mac_provider_serializer *serializer = get_serializer(context, req);
	struct mac_provider *this_instance = (struct mac_provider*)context;

	psa_key_id_t key_id;
	psa_algorithm_t alg;

	if (serializer)
		rpc_status = serializer->deserialize_mac_setup_req(req_buf, &key_id, &alg);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		uint32_t op_handle;

		struct crypto_context *crypto_context =
			crypto_context_pool_alloc(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_MAC, call_req_get_caller_id(req),
				&op_handle);

		if (crypto_context) {

			crypto_context->op.mac = psa_mac_operation_init();

			psa_status_t psa_status =
				(call_req_get_opcode(req) == TS_CRYPTO_OPCODE_MAC_SIGN_SETUP) ?
					psa_mac_sign_setup(&crypto_context->op.mac, key_id, alg) :
					psa_mac_verify_setup(&crypto_context->op.mac, key_id, alg);

			if (psa_status == PSA_SUCCESS) {

				struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
				rpc_status = serializer->serialize_mac_setup_resp(resp_buf, op_handle);
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

static rpc_status_t mac_update_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct mac_provider_serializer *serializer = get_serializer(context, req);
	struct mac_provider *this_instance = (struct mac_provider*)context;

	uint32_t op_handle;
	const uint8_t *data;
	size_t data_len;

	if (serializer)
		rpc_status = serializer->deserialize_mac_update_req(req_buf, &op_handle, &data, &data_len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_MAC, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			psa_status = psa_mac_update(&crypto_context->op.mac, data, data_len);
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t mac_sign_finish_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct mac_provider_serializer *serializer = get_serializer(context, req);
	struct mac_provider *this_instance = (struct mac_provider*)context;

	uint32_t op_handle;

	if (serializer)
		rpc_status = serializer->deserialize_mac_sign_finish_req(req_buf, &op_handle);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_MAC, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			size_t mac_len;
			uint8_t mac[PSA_MAC_MAX_SIZE];

			psa_status = psa_mac_sign_finish(&crypto_context->op.mac, mac, sizeof(mac), &mac_len);

			if (psa_status == PSA_SUCCESS) {

				struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
				rpc_status = serializer->serialize_mac_sign_finish_resp(resp_buf, mac, mac_len);

				crypto_context_pool_free(&this_instance->context_pool, crypto_context);
			}
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t mac_verify_finish_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct mac_provider_serializer *serializer = get_serializer(context, req);
	struct mac_provider *this_instance = (struct mac_provider*)context;

	uint32_t op_handle;
	const uint8_t *mac;
	size_t mac_len;

	if (serializer)
		rpc_status = serializer->deserialize_mac_verify_finish_req(req_buf,
			&op_handle, &mac, &mac_len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t psa_status = PSA_ERROR_BAD_STATE;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_MAC, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			psa_status = psa_mac_verify_finish(&crypto_context->op.mac, mac, mac_len);

			if (psa_status == PSA_SUCCESS) {

				crypto_context_pool_free(&this_instance->context_pool, crypto_context);
			}
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}

static rpc_status_t mac_abort_handler(void *context, struct call_req* req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct mac_provider_serializer *serializer = get_serializer(context, req);
	struct mac_provider *this_instance = (struct mac_provider*)context;

	uint32_t op_handle;

	if (serializer)
		rpc_status = serializer->deserialize_mac_abort_req(req_buf, &op_handle);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		/* Return success if operation is no longer active and
		 * doesn't need aborting.
		 */
		psa_status_t psa_status = PSA_SUCCESS;

		struct crypto_context *crypto_context =
			crypto_context_pool_find(&this_instance->context_pool,
				CRYPTO_CONTEXT_OP_ID_MAC, call_req_get_caller_id(req),
				op_handle);

		if (crypto_context) {

			psa_status = psa_mac_abort(&crypto_context->op.mac);
			crypto_context_pool_free(&this_instance->context_pool, crypto_context);
		}

		call_req_set_opstatus(req, psa_status);
	}

	return rpc_status;
}
