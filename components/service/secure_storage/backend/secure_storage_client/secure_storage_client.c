/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "secure_storage_client.h"
#include <protocols/service/secure_storage/packed-c/secure_storage_proto.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <rpc_caller.h>
#include <string.h>


static psa_status_t secure_storage_client_set(void *context,
			 uint32_t client_id,
			 psa_storage_uid_t uid,
			 size_t data_length,
			 const void *p_data,
			 psa_storage_create_flags_t create_flags)
{
	struct secure_storage_client *this_context = (struct secure_storage_client*)context;
	uint8_t *request;
	uint8_t *response;
	size_t request_length = 0;
	size_t response_length = 0;
	struct secure_storage_request_set *request_desc;
	rpc_call_handle handle;
	rpc_status_t rpc_status = TS_RPC_CALL_ACCEPTED;
	psa_status_t psa_status = PSA_SUCCESS;

	(void)client_id;

	/* Validating input parameters */
	if (p_data == NULL)
		return PSA_ERROR_INVALID_ARGUMENT;

	request_length = sizeof(*request_desc) + data_length;
	if (request_length < data_length) {
		/* size_t overflow */
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	handle = rpc_caller_begin(this_context->rpc_caller, &request, request_length);

	if (handle) {
		/* Populating request descriptor */
		request_desc = (struct secure_storage_request_set *)request;
		request_desc->uid = uid;
		request_desc->data_length = data_length;
		request_desc->create_flags = create_flags;
		memcpy(&request_desc->p_data, p_data, data_length);

		rpc_status = rpc_caller_invoke(this_context->rpc_caller, handle,
						TS_SECURE_STORAGE_OPCODE_SET,
						(uint32_t *)&psa_status, &response,
						&response_length);

		if (rpc_status != TS_RPC_CALL_ACCEPTED) {
			/* RPC failure */
			psa_status = PSA_ERROR_GENERIC_ERROR;
		}

		rpc_caller_end(this_context->rpc_caller, handle);
	}
	else {
		psa_status = PSA_ERROR_GENERIC_ERROR;
	}

	return psa_status;
}

static psa_status_t secure_storage_client_get(void *context,
			 uint32_t client_id,
			 psa_storage_uid_t uid,
			 size_t data_offset,
			 size_t data_size,
			 void *p_data,
			 size_t *p_data_length)
{
	struct secure_storage_client *this_context = (struct secure_storage_client*)context;
	uint8_t *request;
	uint8_t *response;
	size_t response_length = 0;
	struct secure_storage_request_get *request_desc;
	rpc_call_handle handle;
	rpc_status_t rpc_status = TS_RPC_CALL_ACCEPTED;
	psa_status_t psa_status = PSA_SUCCESS;

	(void)client_id;

	/* Validating input parameters */
	if (p_data == NULL)
		return PSA_ERROR_INVALID_ARGUMENT;

	handle = rpc_caller_begin(this_context->rpc_caller, &request, sizeof(*request_desc));

	if (handle) {
		/* Populating request descriptor */
		request_desc = (struct secure_storage_request_get *)request;
		request_desc->uid = uid;
		request_desc->data_offset = data_offset;
		request_desc->data_size = data_size;

		rpc_status = rpc_caller_invoke(this_context->rpc_caller, handle,
						TS_SECURE_STORAGE_OPCODE_GET,
						(uint32_t *)&psa_status, &response,
						&response_length);

		if (rpc_status != TS_RPC_CALL_ACCEPTED ) {
			/* RPC failure */
			psa_status = PSA_ERROR_GENERIC_ERROR;
		}

		/* Filling output parameters */
		if (psa_status == PSA_SUCCESS) {
			*p_data_length = (response_length <= data_size) ? response_length : data_size;
			memcpy(p_data, response, *p_data_length);
		}

		rpc_caller_end(this_context->rpc_caller, handle);
	}
	else {
		psa_status = PSA_ERROR_GENERIC_ERROR;
	}

	return psa_status;
}

static psa_status_t secure_storage_client_get_info(void *context,
				uint32_t client_id,
				psa_storage_uid_t uid,
				struct psa_storage_info_t *p_info)
{
	struct secure_storage_client *this_context = (struct secure_storage_client*)context;
	uint8_t *request;
	uint8_t *response;
	size_t response_length = 0;
	struct secure_storage_request_get_info *request_desc;
	struct secure_storage_response_get_info *response_desc;
	rpc_call_handle handle;
	rpc_status_t rpc_status;
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;

	(void)client_id;

	/* Validating input parameters */
	if (p_info == NULL)
		return PSA_ERROR_INVALID_ARGUMENT;

	handle = rpc_caller_begin(this_context->rpc_caller, &request, sizeof(*request_desc));

	if (handle) {
		/* Populating request descriptor */
		request_desc = (struct secure_storage_request_get_info *)request;
		request_desc->uid = uid;

		rpc_status = rpc_caller_invoke(this_context->rpc_caller, handle,
						TS_SECURE_STORAGE_OPCODE_GET_INFO,
						(uint32_t *)&psa_status, &response,
						&response_length);

		if (rpc_status != TS_RPC_CALL_ACCEPTED) {
			/* RPC failure */
			psa_status = PSA_ERROR_GENERIC_ERROR;
		} else if (response_length && response_length != sizeof(*response_desc)) {
			psa_status = PSA_ERROR_GENERIC_ERROR;
		}

		if (psa_status == PSA_SUCCESS) {
			response_desc = (struct secure_storage_response_get_info *)response;
			p_info->capacity = response_desc->capacity;
			p_info->size = response_desc->size;
			p_info->flags = response_desc->flags;
		} else {
			p_info->capacity = 0;
			p_info->size = 0;
			p_info->flags = PSA_STORAGE_FLAG_NONE;
		}

		rpc_caller_end(this_context->rpc_caller, handle);
	}
	else {
		psa_status = PSA_ERROR_GENERIC_ERROR;
	}

	return psa_status;
}

static psa_status_t secure_storage_client_remove(void *context,
						uint32_t client_id,
						psa_storage_uid_t uid)
{
	struct secure_storage_client *this_context = (struct secure_storage_client*)context;
	uint8_t *request;
	uint8_t *response;
	size_t response_length = 0;
	struct secure_storage_request_remove *request_desc;
	rpc_call_handle handle;
	rpc_status_t rpc_status = TS_RPC_CALL_ACCEPTED;
	psa_status_t psa_status = PSA_SUCCESS;

	(void)client_id;

	handle = rpc_caller_begin(this_context->rpc_caller, &request, sizeof(*request_desc));

	if (handle) {
		/* Populating request descriptor */
		request_desc = (struct secure_storage_request_remove *)request;
		request_desc->uid = uid;

		rpc_status = rpc_caller_invoke(this_context->rpc_caller, handle,
						TS_SECURE_STORAGE_OPCODE_REMOVE,
						(uint32_t *)&psa_status, &response,
						&response_length);

		if (rpc_status != TS_RPC_CALL_ACCEPTED) {
			/* RPC failure */
			psa_status = PSA_ERROR_GENERIC_ERROR;
		}

		rpc_caller_end(this_context->rpc_caller, handle);
	}
	else {
		psa_status = PSA_ERROR_GENERIC_ERROR;
	}

	return psa_status;
}

static psa_status_t secure_storage_client_create(void *context,
                            uint32_t client_id,
                            uint64_t uid,
                            size_t capacity,
                            uint32_t create_flags)
{
	struct secure_storage_client *this_context = (struct secure_storage_client*)context;
	uint8_t *request;
	uint8_t *response;
	size_t request_length = 0;
	size_t response_length = 0;
	struct secure_storage_request_create *request_desc;
	rpc_call_handle handle;
	rpc_status_t rpc_status = TS_RPC_CALL_ACCEPTED;
	psa_status_t psa_status = PSA_SUCCESS;

	(void)client_id;

	request_length = sizeof(*request_desc);

	handle = rpc_caller_begin(this_context->rpc_caller, &request, request_length);

	if (handle) {

		request_desc = (struct secure_storage_request_create*)request;
		request_desc->uid = uid;
		request_desc->capacity = capacity;
		request_desc->create_flags = create_flags;

		rpc_status = rpc_caller_invoke(this_context->rpc_caller, handle,
						TS_SECURE_STORAGE_OPCODE_CREATE,
						(uint32_t *)&psa_status, &response,
						&response_length);

		if (rpc_status != TS_RPC_CALL_ACCEPTED) {
			/* RPC failure */
			psa_status = PSA_ERROR_GENERIC_ERROR;
		}

		rpc_caller_end(this_context->rpc_caller, handle);
	}
	else {
		psa_status = PSA_ERROR_GENERIC_ERROR;
	}

	return psa_status;
}

static psa_status_t secure_storage_set_extended(void *context,
                            uint32_t client_id,
                            uint64_t uid,
                            size_t data_offset,
                            size_t data_length,
                            const void *p_data)
{
	struct secure_storage_client *this_context = (struct secure_storage_client*)context;
	uint8_t *request;
	uint8_t *response;
	size_t request_length = 0;
	size_t response_length = 0;
	struct secure_storage_request_set_extended *request_desc;
	rpc_call_handle handle;
	rpc_status_t rpc_status = TS_RPC_CALL_ACCEPTED;
	psa_status_t psa_status = PSA_SUCCESS;

	(void)client_id;

	/* Validating input parameters */
	if (p_data == NULL)
		return PSA_ERROR_INVALID_ARGUMENT;

	request_length = sizeof(*request_desc) + data_length;
	if (request_length < data_length) {
		/* size_t overflow */
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	handle = rpc_caller_begin(this_context->rpc_caller, &request, request_length);

	if (handle) {
		/* Populating request descriptor */
		request_desc = (struct secure_storage_request_set_extended *)request;
		request_desc->uid = uid;
		request_desc->data_offset = data_offset;
		request_desc->data_length = data_length;
		memcpy(&request_desc->p_data, p_data, data_length);

		rpc_status = rpc_caller_invoke(this_context->rpc_caller, handle,
						TS_SECURE_STORAGE_OPCODE_SET_EXTENDED,
						(uint32_t *)&psa_status, &response,
						&response_length);

		if (rpc_status != TS_RPC_CALL_ACCEPTED) {
			/* RPC failure */
			psa_status = PSA_ERROR_GENERIC_ERROR;
		}

		rpc_caller_end(this_context->rpc_caller, handle);
	}
	else {
		psa_status = PSA_ERROR_GENERIC_ERROR;
	}

	return psa_status;
}

static uint32_t secure_storage_get_support(void *context, uint32_t client_id)
{
	struct secure_storage_client *this_context = (struct secure_storage_client*)context;
	uint8_t *request;
	uint8_t *response;
	size_t response_length = 0;
	struct secure_storage_response_get_support *response_desc;
	rpc_call_handle handle;
	rpc_status_t rpc_status;
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	uint32_t feature_map = 0;

	(void)client_id;

	handle = rpc_caller_begin(this_context->rpc_caller, &request, 0);

	if (handle) {

		rpc_status = rpc_caller_invoke(this_context->rpc_caller, handle,
						TS_SECURE_STORAGE_OPCODE_GET_SUPPORT,
						(uint32_t *)&psa_status, &response,
						&response_length);

		if (rpc_status != TS_RPC_CALL_ACCEPTED) {
			/* RPC failure */
			psa_status = PSA_ERROR_GENERIC_ERROR;
		} else if (response_length < sizeof(*response_desc)) {
			psa_status = PSA_ERROR_GENERIC_ERROR;
		}

		if (psa_status == PSA_SUCCESS) {
			response_desc = (struct secure_storage_response_get_support*)response;
			feature_map = response_desc->support;
		}

		rpc_caller_end(this_context->rpc_caller, handle);
	}

	return feature_map;
}


struct storage_backend *secure_storage_client_init(struct secure_storage_client *context,
								struct rpc_caller *caller)
{
	context->rpc_caller = caller;

	static const struct storage_backend_interface interface =
	{
		secure_storage_client_set,
		secure_storage_client_get,
		secure_storage_client_get_info,
		secure_storage_client_remove,
		secure_storage_client_create,
		secure_storage_set_extended,
		secure_storage_get_support
	};

	context->backend.context = context;
	context->backend.interface = &interface;

	return &context->backend;
}

void secure_storage_client_deinit(struct secure_storage_client *context)
{
	(void)context;
}
