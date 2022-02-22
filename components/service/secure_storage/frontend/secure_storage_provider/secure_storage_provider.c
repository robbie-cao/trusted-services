/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "secure_storage_provider.h"
#include <protocols/service/secure_storage/packed-c/secure_storage_proto.h>
#include <protocols/service/psa/packed-c/status.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <components/rpc/common/endpoint/rpc_interface.h>


static rpc_status_t set_handler(void *context, struct call_req *req)
{
	struct secure_storage_provider *this_context = (struct secure_storage_provider*)context;
	struct secure_storage_request_set *request_desc;
	psa_status_t psa_status;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_set))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_set *)(req->req_buf.data);

	/* Checking for overflow */
	if (sizeof(struct secure_storage_request_set) + request_desc->data_length < request_desc->data_length)
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	/* Checking if descriptor and data fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_set) + request_desc->data_length)
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	psa_status = this_context->backend->interface->set(this_context->backend->context,
				req->caller_id,
				request_desc->uid,
				request_desc->data_length,
				request_desc->p_data,
				request_desc->create_flags);
	call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t get_handler(void *context, struct call_req *req)
{
	struct secure_storage_provider *this_context = (struct secure_storage_provider*)context;
	struct secure_storage_request_get *request_desc;
	psa_status_t psa_status;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_get))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_get *)(req->req_buf.data);

	/* Clip the requested data size if it's too big for the response buffer */
	size_t data_size = (req->resp_buf.size < request_desc->data_size) ?
		req->resp_buf.size :
		request_desc->data_size;

	psa_status = this_context->backend->interface->get(this_context->backend->context,
				req->caller_id, request_desc->uid,
				request_desc->data_offset,
				data_size,
				req->resp_buf.data, &req->resp_buf.data_len);
	call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t get_info_handler(void *context, struct call_req *req)
{
	struct secure_storage_provider *this_context = (struct secure_storage_provider*)context;
	struct secure_storage_request_get_info *request_desc;
	struct secure_storage_response_get_info *response_desc;
	struct psa_storage_info_t storage_info;
	psa_status_t psa_status;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_get_info))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_get_info *)(req->req_buf.data);

	/* Checking if the response structure would fit the response buffer */
	if (req->resp_buf.size < sizeof(struct secure_storage_response_get_info))
		return TS_RPC_ERROR_INVALID_RESP_BODY;

	response_desc = (struct secure_storage_response_get_info *)(req->resp_buf.data);

	psa_status = this_context->backend->interface->get_info(this_context->backend->context,
				req->caller_id,
				request_desc->uid,
				&storage_info);
	call_req_set_opstatus(req, psa_status);

	if (psa_status != PSA_SUCCESS) {
		req->resp_buf.data_len = 0;
	}
	else {
		response_desc->capacity = storage_info.capacity;
		response_desc->size = storage_info.size;
		response_desc->flags = storage_info.flags;

		req->resp_buf.data_len = sizeof(struct secure_storage_response_get_info);
	}

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t remove_handler(void *context, struct call_req *req)
{
	struct secure_storage_provider *this_context = (struct secure_storage_provider*)context;
	struct secure_storage_request_remove *request_desc;
	psa_status_t psa_status;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_remove))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_remove *)(req->req_buf.data);

	psa_status = this_context->backend->interface->remove(this_context->backend->context,
				req->caller_id,
				request_desc->uid);
	call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t create_handler(void *context, struct call_req *req)
{
	struct secure_storage_provider *this_context = (struct secure_storage_provider*)context;
	struct secure_storage_request_create *request_desc;
	psa_status_t psa_status;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_create))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_create *)(req->req_buf.data);

	psa_status = this_context->backend->interface->create(this_context->backend->context,
				req->caller_id,
				request_desc->uid,
				request_desc->capacity,
				request_desc->create_flags);
	call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t set_extended_handler(void *context, struct call_req *req)
{
	struct secure_storage_provider *this_context = (struct secure_storage_provider*)context;
	struct secure_storage_request_set_extended *request_desc;
	psa_status_t psa_status;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_set_extended))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_set_extended *)(req->req_buf.data);

	/* Checking for overflow */
	if (sizeof(struct secure_storage_request_set_extended) + request_desc->data_length < request_desc->data_length)
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	/* Checking if descriptor and data fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_set_extended) + request_desc->data_length)
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	psa_status = this_context->backend->interface->set_extended(this_context->backend->context,
				req->caller_id,
				request_desc->uid,
				request_desc->data_offset,
				request_desc->data_length,
				request_desc->p_data);
	call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t get_support_handler(void *context, struct call_req *req)
{
	struct secure_storage_provider *this_context = (struct secure_storage_provider*)context;
	struct secure_storage_response_get_support *response_desc;
	uint32_t feature_map;

	/* Checking if the response structure would fit the response buffer */
	if (req->resp_buf.size < sizeof(struct secure_storage_response_get_support))
		return TS_RPC_ERROR_INVALID_RESP_BODY;

	response_desc = (struct secure_storage_response_get_support *)(req->resp_buf.data);

	feature_map = this_context->backend->interface->get_support(this_context->backend->context,
				req->caller_id);
	call_req_set_opstatus(req, PSA_SUCCESS);

	response_desc->support = feature_map;
	req->resp_buf.data_len = sizeof(struct secure_storage_response_get_support);

	return TS_RPC_CALL_ACCEPTED;
}

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{TS_SECURE_STORAGE_OPCODE_SET,	set_handler},
	{TS_SECURE_STORAGE_OPCODE_GET,	get_handler},
	{TS_SECURE_STORAGE_OPCODE_GET_INFO,	get_info_handler},
	{TS_SECURE_STORAGE_OPCODE_REMOVE,	remove_handler},
	{TS_SECURE_STORAGE_OPCODE_CREATE,	create_handler},
	{TS_SECURE_STORAGE_OPCODE_SET_EXTENDED,	set_extended_handler},
	{TS_SECURE_STORAGE_OPCODE_GET_SUPPORT,	get_support_handler}
};

struct rpc_interface *secure_storage_provider_init(struct secure_storage_provider *context,
												struct storage_backend *backend)
{
	struct rpc_interface *rpc_interface = NULL;

	if (context == NULL)
		goto out;

	if (backend == NULL)
		goto out;

	service_provider_init(&context->base_provider, context, handler_table,
				  sizeof(handler_table) / sizeof(handler_table[0]));

	rpc_interface = service_provider_get_rpc_interface(&context->base_provider);

	context->backend = backend;

out:
	return rpc_interface;
}

void secure_storage_provider_deinit(struct secure_storage_provider *context)
{
	(void)context;
}
