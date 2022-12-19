/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <common/uuid/uuid.h>
#include <protocols/service/fwu/packed-c/opcodes.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <service/fwu/agent/update_agent.h>
#include <service/fwu/provider/serializer/fwu_provider_serializer.h>
#include "fwu_provider.h"

/* Service request handlers */
static rpc_status_t begin_staging_handler(void *context, struct call_req *req);
static rpc_status_t end_staging_handler(void *context, struct call_req *req);
static rpc_status_t cancel_staging_handler(void *context, struct call_req *req);
static rpc_status_t open_handler(void *context, struct call_req *req);
static rpc_status_t write_stream_handler(void *context, struct call_req *req);
static rpc_status_t read_stream_handler(void *context, struct call_req *req);
static rpc_status_t commit_handler(void *context, struct call_req *req);
static rpc_status_t accept_image_handler(void *context, struct call_req *req);
static rpc_status_t select_previous_handler(void *context, struct call_req *req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{TS_FWU_OPCODE_BEGIN_STAGING,     begin_staging_handler},
	{TS_FWU_OPCODE_END_STAGING,       end_staging_handler},
	{TS_FWU_OPCODE_CANCEL_STAGING,    cancel_staging_handler},
	{TS_FWU_OPCODE_OPEN,              open_handler},
	{TS_FWU_OPCODE_WRITE_STREAM,      write_stream_handler},
	{TS_FWU_OPCODE_READ_STREAM,       read_stream_handler},
	{TS_FWU_OPCODE_COMMIT,            commit_handler},
	{TS_FWU_OPCODE_ACCEPT_IMAGE,      accept_image_handler},
	{TS_FWU_OPCODE_SELECT_PREVIOUS,   select_previous_handler}
};

struct rpc_interface *fwu_provider_init(
	struct fwu_provider *context,
	struct update_agent *update_agent)
{
	/* Initialise the fwu_provider */
	context->update_agent = update_agent;

	for (size_t encoding = 0; encoding < TS_RPC_ENCODING_LIMIT; ++encoding)
		context->serializers[encoding] = NULL;

	service_provider_init(&context->base_provider, context,
		handler_table, sizeof(handler_table)/sizeof(struct service_handler));

	/* Initialise the associated discovery_provider and attach it */
	discovery_provider_init(&context->discovery_provider);
	service_provider_extend(
		&context->base_provider,
		&context->discovery_provider.base_provider);

	return service_provider_get_rpc_interface(&context->base_provider);
}

void fwu_provider_deinit(
	struct fwu_provider *context)
{
	discovery_provider_deinit(&context->discovery_provider);
}

void fwu_provider_register_serializer(
	struct fwu_provider *context,
	unsigned int encoding,
	const struct fwu_provider_serializer *serializer)
{
	if (encoding < TS_RPC_ENCODING_LIMIT) {

		context->serializers[encoding] = serializer;
		discovery_provider_register_supported_encoding(
			&context->discovery_provider, encoding);
	}
}

static const struct fwu_provider_serializer* get_fwu_serializer(
	struct fwu_provider *this_instance,
	const struct call_req *req)
{
	const struct fwu_provider_serializer* serializer = NULL;
	unsigned int encoding = call_req_get_encoding(req);

	if (encoding < TS_RPC_ENCODING_LIMIT)
		serializer = this_instance->serializers[encoding];

	return serializer;
}

static rpc_status_t begin_staging_handler(void *context, struct call_req *req)
{
	struct fwu_provider *this_instance = (struct fwu_provider *)context;

	int op_status = update_agent_begin_staging(this_instance->update_agent);

	call_req_set_opstatus(req, op_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t end_staging_handler(void *context, struct call_req *req)
{
	struct fwu_provider *this_instance = (struct fwu_provider *)context;

	int op_status = update_agent_end_staging(this_instance->update_agent);

	call_req_set_opstatus(req, op_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t cancel_staging_handler(void *context, struct call_req *req)
{
	struct fwu_provider *this_instance = (struct fwu_provider *)context;

	int op_status = update_agent_cancel_staging(this_instance->update_agent);

	call_req_set_opstatus(req, op_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t open_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	struct fwu_provider *this_instance = (struct fwu_provider *)context;
	const struct fwu_provider_serializer *serializer = get_fwu_serializer(this_instance, req);
	struct uuid_octets image_type_uuid;

	if (serializer)
		rpc_status = serializer->deserialize_open_req(req_buf, &image_type_uuid);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		uint32_t handle = 0;
		int op_status = update_agent_open(this_instance->update_agent,
			&image_type_uuid, &handle);

		if (!op_status) {

			struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
			rpc_status = serializer->serialize_open_resp(resp_buf, handle);
		}

		call_req_set_opstatus(req, op_status);
	}

	return rpc_status;
}

static rpc_status_t write_stream_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	struct fwu_provider *this_instance = (struct fwu_provider *)context;
	const struct fwu_provider_serializer *serializer = get_fwu_serializer(this_instance, req);
	uint32_t handle = 0;
	size_t data_len = 0;
	const uint8_t *data = NULL;

	if (serializer)
		rpc_status = serializer->deserialize_write_stream_req(
			req_buf, &handle, &data_len, &data);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		int op_status = update_agent_write_stream(this_instance->update_agent,
			handle, data, data_len);

		call_req_set_opstatus(req, op_status);
	}

	return rpc_status;
}

static rpc_status_t read_stream_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	struct fwu_provider *this_instance = (struct fwu_provider *)context;
	const struct fwu_provider_serializer *serializer = get_fwu_serializer(this_instance, req);
	uint32_t handle = 0;

	if (serializer)
		rpc_status = serializer->deserialize_read_stream_req(req_buf, &handle);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
		uint8_t *payload_buf;
		size_t max_payload;
		size_t read_len = 0;
		size_t total_len = 0;

		serializer->read_stream_resp_payload(resp_buf, &payload_buf, &max_payload);

		int op_status = update_agent_read_stream(this_instance->update_agent,
			handle, payload_buf, max_payload,
			&read_len, &total_len);

		if (!op_status)
			rpc_status = serializer->serialize_read_stream_resp(resp_buf,
				read_len, total_len);

		call_req_set_opstatus(req, op_status);
	}

	return rpc_status;
}

static rpc_status_t commit_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	struct fwu_provider *this_instance = (struct fwu_provider *)context;
	const struct fwu_provider_serializer *serializer = get_fwu_serializer(this_instance, req);
	uint32_t handle = 0;
	bool accepted = false;
	size_t max_atomic_len = 0;

	if (serializer)
		rpc_status = serializer->deserialize_commit_req(req_buf,
			&handle, &accepted, &max_atomic_len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		int op_status = update_agent_commit(
			this_instance->update_agent, handle, accepted);

		if (!op_status) {

			struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
			rpc_status = serializer->serialize_commit_resp(resp_buf, 0, 0);
		}

		call_req_set_opstatus(req, op_status);
	}

	return rpc_status;
}

static rpc_status_t accept_image_handler(void *context, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	struct fwu_provider *this_instance = (struct fwu_provider *)context;
	const struct fwu_provider_serializer *serializer = get_fwu_serializer(this_instance, req);
	struct uuid_octets image_type_uuid;

	if (serializer)
		rpc_status = serializer->deserialize_accept_req(req_buf, &image_type_uuid);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		int op_status = update_agent_accept(this_instance->update_agent, &image_type_uuid);

		call_req_set_opstatus(req, op_status);
	}

	return rpc_status;
}

static rpc_status_t select_previous_handler(void *context, struct call_req *req)
{
	struct fwu_provider *this_instance = (struct fwu_provider *)context;

	int op_status = update_agent_select_previous(this_instance->update_agent);

	call_req_set_opstatus(req, op_status);

	return TS_RPC_CALL_ACCEPTED;
}
