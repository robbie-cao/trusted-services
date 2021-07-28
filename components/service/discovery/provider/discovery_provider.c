/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <protocols/service/discovery/packed-c/opcodes.h>
#include <service/discovery/provider/discovery_provider.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <psa/error.h>

/* Service request handlers */
static rpc_status_t get_service_info_handler(void *context, struct call_req* req);
static rpc_status_t get_provider_info_handler(void *context, struct call_req* req);
static rpc_status_t get_service_caps_handler(void *context, struct call_req* req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{TS_DISCOVERY_OPCODE_GET_SERVICE_INFO,          get_service_info_handler},
	{TS_DISCOVERY_OPCODE_GET_PROVIDER_INFO,         get_provider_info_handler},
	{TS_DISCOVERY_OPCODE_GET_SERVICE_CAPS,          get_service_caps_handler}
};


struct rpc_interface *discovery_provider_init(
	struct discovery_provider *context,
	const struct discovery_deployment_info *deployment_info,
	const struct discovery_identity_info *identity_info)
{
	for (size_t encoding = 0; encoding < TS_RPC_ENCODING_LIMIT; ++encoding)
		context->serializers[encoding] = NULL;

	service_provider_init(&context->base_provider, context,
					handler_table, sizeof(handler_table)/sizeof(struct service_handler));

	context->info.deployment = *deployment_info;
	context->info.identity = *identity_info;

	context->info.supported_encodings = 0;

	return service_provider_get_rpc_interface(&context->base_provider);
}

void discovery_provider_deinit(struct discovery_provider *context)
{
	(void)context;
}

void discovery_provider_register_serializer(struct discovery_provider *context,
				unsigned int encoding, const struct discovery_provider_serializer *serializer)
{
	if (encoding < TS_RPC_ENCODING_LIMIT)
		context->serializers[encoding] = serializer;
}

void discovery_provider_register_supported_encoding(
	struct discovery_provider *context,
	unsigned int encoding)
{
	context->info.supported_encodings |= (1U << encoding);
}

static const struct discovery_provider_serializer* get_discovery_serializer(void *context,
														const struct call_req *req)
{
	struct discovery_provider *this_instance = (struct discovery_provider*)context;
	const struct discovery_provider_serializer* serializer = NULL;
	unsigned int encoding = call_req_get_encoding(req);

	if (encoding < TS_RPC_ENCODING_LIMIT) serializer = this_instance->serializers[encoding];

	return serializer;
}

static rpc_status_t get_service_info_handler(void *context, struct call_req* req)
{
	struct discovery_provider *this_instance = (struct discovery_provider*)context;
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	const struct discovery_provider_serializer *serializer = get_discovery_serializer(context, req);

	if (serializer) {

		struct call_param_buf *resp_buf = call_req_get_resp_buf(req);

		rpc_status = serializer->serialize_get_service_info_resp(resp_buf, &this_instance->info);
		call_req_set_opstatus(req, PSA_SUCCESS);
	}

	return rpc_status;
}

static rpc_status_t get_provider_info_handler(void *context, struct call_req* req)
{
	(void)context;
	(void)req;
	call_req_set_opstatus(req, PSA_ERROR_NOT_SUPPORTED);
	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t get_service_caps_handler(void *context, struct call_req* req)
{
	(void)context;
	(void)req;
	call_req_set_opstatus(req, PSA_ERROR_NOT_SUPPORTED);
	return TS_RPC_CALL_ACCEPTED;
}
