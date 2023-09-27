/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "direct_caller.h"
#include "components/rpc/common/endpoint/rpc_service_interface.h"
#include "components/rpc/common/interface/rpc_uuid.h"
#include <stdlib.h>

struct direct_caller_context {
	struct rpc_service_interface *service;
};

static rpc_status_t find_and_open_session(void *context, const struct rpc_uuid *service_uuid);

static rpc_status_t open_session(void *context, const struct rpc_uuid *service_uuid,
				 uint16_t endpoint_id)
{
	(void)endpoint_id;

	return find_and_open_session(context, service_uuid);
}

static rpc_status_t find_and_open_session(void *context, const struct rpc_uuid *service_uuid)
{
	struct direct_caller_context *caller = (struct direct_caller_context *)context;

	if (!rpc_uuid_equal(service_uuid, &caller->service->uuid))
		return RPC_ERROR_NOT_FOUND;

	return RPC_SUCCESS;
}

static rpc_status_t close_session(void *context)
{
	return RPC_SUCCESS;
}

static rpc_status_t create_shared_memory(void *context, size_t size,
					 struct rpc_caller_shared_memory *shared_memory)
{
	shared_memory->id = 0;
	shared_memory->buffer = calloc(1, size);
	shared_memory->size = size;

	return RPC_SUCCESS;
}

static rpc_status_t release_shared_memory(void *context,
					  struct rpc_caller_shared_memory *shared_memory)
{
	free(shared_memory->buffer);

	return RPC_SUCCESS;
}

static rpc_status_t call(void *context, uint16_t opcode,
			 struct rpc_caller_shared_memory *shared_memory, size_t request_length,
			 size_t *response_length, service_status_t *service_status)
{
	struct direct_caller_context *caller = (struct direct_caller_context *)context;
	struct rpc_request rpc_request = { 0 };
	rpc_status_t status = RPC_ERROR_INTERNAL;

	rpc_request.source_id = 0;
	rpc_request.opcode = opcode;
	rpc_request.client_id = 0;
	rpc_request.request.data = shared_memory->buffer;
	rpc_request.request.data_length = request_length;
	rpc_request.request.size = shared_memory->size;
	rpc_request.response.data = shared_memory->buffer;
	rpc_request.response.data_length = 0;
	rpc_request.response.size = shared_memory->size;

	status = rpc_service_receive(caller->service, &rpc_request);

	*response_length = rpc_request.response.data_length;
	*service_status = rpc_request.service_status;

	return status;
}

rpc_status_t direct_caller_init(struct rpc_caller_interface *caller,
				struct rpc_service_interface *service)
{
	struct direct_caller_context *context = NULL;

	if (!caller || caller->context)
		return RPC_ERROR_INVALID_VALUE;

	context = (struct direct_caller_context *)calloc(1, sizeof(struct direct_caller_context));
	if (!context)
		return RPC_ERROR_INTERNAL;

	context->service = service;

	caller->context = context;
	caller->open_session = open_session;
	caller->find_and_open_session = find_and_open_session;
	caller->close_session = close_session;
	caller->create_shared_memory = create_shared_memory;
	caller->release_shared_memory = release_shared_memory;
	caller->call = call;

	return RPC_SUCCESS;
}

rpc_status_t direct_caller_deinit(struct rpc_caller_interface *rpc_caller)
{
	if (!rpc_caller || !rpc_caller->context)
		return RPC_ERROR_INVALID_VALUE;

	free(rpc_caller->context);

	return RPC_SUCCESS;
}
