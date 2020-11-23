/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sfs_provider.h"
#include "secure_flash_store.h"
#include <protocols/service/secure_storage/packed-c/secure_storage_proto.h>
#include <protocols/service/psa/packed-c/status.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <components/rpc/common/endpoint/call_ep.h>

#include <stdio.h>

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{TS_SECURE_STORAGE_OPCODE_SET,	sfs_set_handler},
	{TS_SECURE_STORAGE_OPCODE_GET,	sfs_get_handler},
	{TS_SECURE_STORAGE_OPCODE_GET_INFO,	sfs_get_info_handler},
	{TS_SECURE_STORAGE_OPCODE_REMOVE,	sfs_remove_handler}
};

struct call_ep *sfs_provider_init(struct sfs_provider *context)
{
	struct call_ep *call_ep = NULL;

	if (context == NULL)
		goto out;

	if (sfs_init() != PSA_SUCCESS)
		goto out;

	service_provider_init(&context->base_provider, context, handler_table,
			      sizeof(handler_table) / sizeof(handler_table[0]));

	call_ep = service_provider_get_call_ep(&context->base_provider);

out:
	return call_ep;
}

rpc_status_t sfs_set_handler(void *context, struct call_req *req)
{
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

	psa_status = sfs_set(req->caller_id, request_desc->uid,
				 request_desc->data_length,
				 request_desc->p_data,
				 request_desc->create_flags);
	call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}

rpc_status_t sfs_get_handler(void *context, struct call_req *req)
{
	struct secure_storage_request_get *request_desc;
	psa_status_t psa_status;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_get))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_get *)(req->req_buf.data);

	/* Check if the requested data would fit into the response buffer. */
	if (req->resp_buf.size < request_desc->data_size)
		return TS_RPC_ERROR_INVALID_RESP_BODY;

	psa_status = sfs_get(req->caller_id, request_desc->uid,
				 request_desc->data_offset,
				 request_desc->data_size,
				 req->resp_buf.data, &req->resp_buf.data_len);
	call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}

rpc_status_t sfs_get_info_handler(void *context, struct call_req *req)
{
	struct secure_storage_request_get_info *request_desc;
	struct secure_storage_response_get_info *response_desc;
	struct secure_storage_response_get_info storage_info; //TODO: unnecessary?
	psa_status_t psa_status;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_get_info))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_get_info *)(req->req_buf.data);

	/* Checking if the response structure would fit the response buffer */
	if (req->resp_buf.size < sizeof(struct secure_storage_response_get_info))
		return TS_RPC_ERROR_INVALID_RESP_BODY;

	response_desc = (struct secure_storage_response_get_info *)(req->resp_buf.data);

	psa_status = sfs_get_info(req->caller_id, request_desc->uid, &storage_info);
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

rpc_status_t sfs_remove_handler(void *context, struct call_req *req)
{
	struct secure_storage_request_remove *request_desc;
	psa_status_t psa_status;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_remove))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_remove *)(req->req_buf.data);

	psa_status = sfs_remove(req->caller_id, request_desc->uid);
	call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}
