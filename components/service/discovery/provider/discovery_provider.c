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
static rpc_status_t get_service_info_handler(void *context, struct call_req *req);
static rpc_status_t get_provider_info_handler(void *context, struct call_req *req);
static rpc_status_t get_service_caps_handler(void *context, struct call_req *req);

/* Other private functions */
static size_t determine_max_payload(
	const struct discovery_deployment_info *deployment_info,
	const struct call_req *req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{TS_DISCOVERY_OPCODE_GET_SERVICE_INFO,          get_service_info_handler},
	{TS_DISCOVERY_OPCODE_GET_PROVIDER_INFO,         get_provider_info_handler},
	{TS_DISCOVERY_OPCODE_GET_SERVICE_CAPS,          get_service_caps_handler}
};


void discovery_provider_init(struct discovery_provider *context)
{
	/* Initialise the base provider */
	for (size_t encoding = 0; encoding < TS_RPC_ENCODING_LIMIT; ++encoding)
		context->serializers[encoding] = NULL;

	service_provider_init(&context->base_provider, context,
					handler_table, sizeof(handler_table)/sizeof(struct service_handler));

	/* Set default deployment settings.  Deployment specific settings may be
	 * applied using discovery_provider_set_deployment_info().
	 */
	context->info.deployment.interface_id = 0;
	context->info.deployment.instance = 0;
	context->info.deployment.max_payload_override = 0;

	/* Bitmap is set when serializers are registered */
	context->info.supported_encodings = 0;
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

void discovery_provider_set_deployment_info(
	struct discovery_provider *context,
	const struct discovery_deployment_info *deployment_info)
{
	context->info.deployment = *deployment_info;
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

		size_t max_payload = determine_max_payload(&this_instance->info.deployment, req);
		struct call_param_buf *resp_buf = call_req_get_resp_buf(req);

		rpc_status = serializer->serialize_get_service_info_resp(resp_buf,
			max_payload,
			&this_instance->info);

		call_req_set_opstatus(req, PSA_SUCCESS);
	}

	return rpc_status;
}

static rpc_status_t get_provider_info_handler(void *context, struct call_req* req)
{
	(void)context;

	call_req_set_opstatus(req, PSA_ERROR_NOT_SUPPORTED);
	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t get_service_caps_handler(void *context, struct call_req* req)
{
	(void)context;

	call_req_set_opstatus(req, PSA_ERROR_NOT_SUPPORTED);
	return TS_RPC_CALL_ACCEPTED;
}

static size_t determine_max_payload(
	const struct discovery_deployment_info *deployment_info,
	const struct call_req *req)
{
	size_t max_payload;

	if (!deployment_info->max_payload_override) {

		/* No deployment specific override has been provided so
		 * determine the maximum payload value from the call_req
		 * buffer sizes.  This will have been set by the
		 * underlying RPC layer.
		 */
		const struct call_param_buf *req_buf = &req->req_buf;
		const struct call_param_buf *resp_buf = &req->resp_buf;

		max_payload = (req_buf->size < resp_buf->size) ?
			req_buf->size :
			resp_buf->size;
	}
	else {

		/* A deployment specific override has been provided */
		max_payload = deployment_info->max_payload_override;
	}

	return max_payload;
}
