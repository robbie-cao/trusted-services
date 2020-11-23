/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "its_client.h"
#include <psa/internal_trusted_storage.h>
#include <protocols/service/secure_storage/packed-c/secure_storage_proto.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <assert.h>
#include <string.h>

/* Variables */
static struct rpc_caller *rpc_caller;

psa_status_t psa_its_client_init(struct rpc_caller *caller)
{
	rpc_caller = caller;

	return PSA_SUCCESS;
}

psa_status_t psa_its_set(psa_storage_uid_t uid,
			 size_t data_length,
			 const void *p_data,
			 psa_storage_create_flags_t create_flags)
{
	uint8_t *request;
	uint8_t *response;
	size_t request_length = 0;
	size_t response_length = 0;
	struct secure_storage_request_set *request_desc;
	rpc_call_handle handle;
	rpc_status_t rpc_status = TS_RPC_CALL_ACCEPTED;
	psa_status_t psa_status = PSA_SUCCESS;

	/* Validating input parameters */
	if (p_data == NULL)
		return PSA_ERROR_INVALID_ARGUMENT;

	request_length = sizeof(*request_desc) + data_length;
	if (request_length < data_length) {
		/* size_t overflow */
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	handle = rpc_caller_begin(rpc_caller, &request, request_length);

	if (handle) {
		/* Populating request descriptor */
		request_desc = (struct secure_storage_request_set *)request;
		request_desc->uid = uid;
		request_desc->data_length = data_length;
		request_desc->create_flags = create_flags;
		memcpy(&request_desc->p_data, p_data, data_length);

		rpc_status = rpc_caller_invoke(rpc_caller, handle, TS_SECURE_STORAGE_OPCODE_SET,
						(uint32_t *)&psa_status, &response,
						&response_length);

		if (rpc_status != TS_RPC_CALL_ACCEPTED) {
			/* RPC failure */
			psa_status = PSA_ERROR_GENERIC_ERROR;
		}

		rpc_caller_end(rpc_caller, handle);
	}
	else {
		psa_status = PSA_ERROR_GENERIC_ERROR;
	}

	return psa_status;
}

psa_status_t psa_its_get(psa_storage_uid_t uid,
			 size_t data_offset,
			 size_t data_size,
			 void *p_data,
			 size_t *p_data_length)
{
	uint8_t *request;
	uint8_t *response;
	size_t response_length = 0;
	struct secure_storage_request_get *request_desc;
	rpc_call_handle handle;
	rpc_status_t rpc_status = TS_RPC_CALL_ACCEPTED;
	psa_status_t psa_status = PSA_SUCCESS;

	/* Validating input parameters */
	if (p_data == NULL)
		return PSA_ERROR_INVALID_ARGUMENT;

	handle = rpc_caller_begin(rpc_caller, &request, sizeof(*request_desc));

	if (handle) {
		/* Populating request descriptor */
		request_desc = (struct secure_storage_request_get *)request;
		request_desc->uid = uid;
		request_desc->data_offset = data_offset;
		request_desc->data_size = data_size;

		rpc_status = rpc_caller_invoke(rpc_caller, handle, TS_SECURE_STORAGE_OPCODE_GET,
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

		rpc_caller_end(rpc_caller, handle);
	}
	else {
		psa_status = PSA_ERROR_GENERIC_ERROR;
	}

	return psa_status;
}

psa_status_t psa_its_get_info(psa_storage_uid_t uid,
			      struct psa_storage_info_t *p_info)
{
	uint8_t *request;
	uint8_t *response;
	size_t response_length = 0;
	struct secure_storage_request_get_info *request_desc;
	struct secure_storage_response_get_info *response_desc;
	rpc_call_handle handle;
	rpc_status_t rpc_status;
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;

	/* Validating input parameters */
	if (p_info == NULL)
		return PSA_ERROR_INVALID_ARGUMENT;

	handle = rpc_caller_begin(rpc_caller, &request, sizeof(*request_desc));

	if (handle) {
		/* Populating request descriptor */
		request_desc = (struct secure_storage_request_get_info *)request;
		request_desc->uid = uid;

		rpc_status = rpc_caller_invoke(rpc_caller, handle,
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

		rpc_caller_end(rpc_caller, handle);
	}
	else {
		psa_status = PSA_ERROR_GENERIC_ERROR;
	}

	return psa_status;
}

psa_status_t psa_its_remove(psa_storage_uid_t uid)
{
	uint8_t *request;
	uint8_t *response;
	size_t response_length = 0;
	struct secure_storage_request_remove *request_desc;
	rpc_call_handle handle;
	rpc_status_t rpc_status = TS_RPC_CALL_ACCEPTED;
	psa_status_t psa_status = PSA_SUCCESS;

	handle = rpc_caller_begin(rpc_caller, &request, sizeof(*request_desc));

	if (handle) {
		/* Populating request descriptor */
		request_desc = (struct secure_storage_request_remove *)request;
		request_desc->uid = uid;

		rpc_status = rpc_caller_invoke(rpc_caller, handle, TS_SECURE_STORAGE_OPCODE_REMOVE,
						(uint32_t *)&psa_status, &response,
						&response_length);

		if (rpc_status != TS_RPC_CALL_ACCEPTED) {
			/* RPC failure */
			psa_status = PSA_ERROR_GENERIC_ERROR;
		}

		rpc_caller_end(rpc_caller, handle);
	}
	else {
		psa_status = PSA_ERROR_GENERIC_ERROR;
	}

	return psa_status;
}
