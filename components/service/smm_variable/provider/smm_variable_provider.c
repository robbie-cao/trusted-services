/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <string.h>
#include <protocols/service/smm_variable/smm_variable_proto.h>
#include <protocols/rpc/common/packed-c/status.h>
#include "smm_variable_provider.h"

/* Service request handlers */
static rpc_status_t get_variable_handler(void *context, struct call_req *req);
static rpc_status_t get_next_variable_name_handler(void *context, struct call_req *req);
static rpc_status_t set_variable_handler(void *context, struct call_req *req);
static rpc_status_t query_variable_info_handler(void *context, struct call_req *req);
static rpc_status_t exit_boot_service_handler(void *context, struct call_req *req);
static rpc_status_t set_var_check_property_handler(void *context, struct call_req *req);
static rpc_status_t get_var_check_property_handler(void *context, struct call_req *req);
static rpc_status_t get_payload_size_handler(void *context, struct call_req *req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{SMM_VARIABLE_FUNCTION_GET_VARIABLE,						get_variable_handler},
	{SMM_VARIABLE_FUNCTION_GET_NEXT_VARIABLE_NAME,				get_next_variable_name_handler},
	{SMM_VARIABLE_FUNCTION_SET_VARIABLE,						set_variable_handler},
	{SMM_VARIABLE_FUNCTION_QUERY_VARIABLE_INFO,					query_variable_info_handler},
	{SMM_VARIABLE_FUNCTION_EXIT_BOOT_SERVICE,					exit_boot_service_handler},
	{SMM_VARIABLE_FUNCTION_VAR_CHECK_VARIABLE_PROPERTY_SET,		set_var_check_property_handler},
	{SMM_VARIABLE_FUNCTION_VAR_CHECK_VARIABLE_PROPERTY_GET,		get_var_check_property_handler},
	{SMM_VARIABLE_FUNCTION_GET_PAYLOAD_SIZE,					get_payload_size_handler}
};

struct rpc_interface *smm_variable_provider_init(
	struct smm_variable_provider *context,
 	uint32_t owner_id,
	size_t max_variables,
	struct storage_backend *persistent_store,
	struct storage_backend *volatile_store)
{
	struct rpc_interface *rpc_interface = NULL;

	if (context) {

		service_provider_init(&context->base_provider, context,
					handler_table, sizeof(handler_table)/sizeof(struct service_handler));

		if (uefi_variable_store_init(
				&context->variable_store,
				owner_id,
				max_variables,
				persistent_store,
				volatile_store) == EFI_SUCCESS) {

			rpc_interface = service_provider_get_rpc_interface(&context->base_provider);
		}
	}

	return rpc_interface;
}

void smm_variable_provider_deinit(struct smm_variable_provider *context)
{
	uefi_variable_store_deinit(&context->variable_store);
}

static efi_status_t sanitize_access_variable_param(struct call_req *req, size_t *param_len)
{
	efi_status_t efi_status = EFI_INVALID_PARAMETER;
	*param_len = 0;
	const struct call_param_buf *req_buf = call_req_get_req_buf(req);

	if (req_buf->data_len >= SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_NAME_OFFSET) {

		const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *param =
			(const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE*)req_buf->data;

		size_t max_space_for_name = req_buf->data_len -
			SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_NAME_OFFSET;

		if (param->NameSize <= max_space_for_name) {

			*param_len = SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_DATA_OFFSET(param);
			efi_status = EFI_SUCCESS;
		}
	}

	return efi_status;
}

static efi_status_t sanitize_get_next_var_name_param(struct call_req *req, size_t *param_len)
{
	efi_status_t efi_status = EFI_INVALID_PARAMETER;
	*param_len = 0;
	const struct call_param_buf *req_buf = call_req_get_req_buf(req);

	if (req_buf->data_len >= SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_NAME_OFFSET) {

		const SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME *param =
			(const SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME*)req_buf->data;

		size_t max_space_for_name = req_buf->data_len -
			SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_NAME_OFFSET;

		if (param->NameSize <= max_space_for_name) {

			*param_len = SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_TOTAL_SIZE(param);
			efi_status = EFI_SUCCESS;
		}
	}

	return efi_status;
}

static efi_status_t sanitize_var_check_property_param(struct call_req *req, size_t *param_len)
{
	efi_status_t efi_status = EFI_INVALID_PARAMETER;
	*param_len = 0;
	const struct call_param_buf *req_buf = call_req_get_req_buf(req);

	if (req_buf->data_len >= SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY_NAME_OFFSET) {

		const SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY *param =
			(const SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY*)req_buf->data;

		size_t max_space_for_name = req_buf->data_len -
			SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY_NAME_OFFSET;

		if (param->NameSize <= max_space_for_name) {

			*param_len = SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY_TOTAL_SIZE(param);
			efi_status = EFI_SUCCESS;
		}
	}

	return efi_status;
}

static rpc_status_t get_variable_handler(void *context, struct call_req *req)
{
	struct smm_variable_provider *this_instance = (struct smm_variable_provider*)context;

	size_t param_len = 0;
	efi_status_t efi_status = sanitize_access_variable_param(req, &param_len);

	if (efi_status == EFI_SUCCESS) {

		/* Valid access variable header parameter */
		struct call_param_buf *resp_buf = call_req_get_resp_buf(req);

		if (resp_buf->size >= param_len) {

			struct call_param_buf *req_buf = call_req_get_req_buf(req);
			size_t max_data_len = resp_buf->size - param_len;

			memmove(resp_buf->data, req_buf->data, param_len);

			efi_status = uefi_variable_store_get_variable(
				&this_instance->variable_store,
				(SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE*)resp_buf->data,
				max_data_len,
				&resp_buf->data_len);
		}
		else {

			/* Reponse buffer not big enough */
			efi_status = EFI_BAD_BUFFER_SIZE;
		}
	}

	call_req_set_opstatus(req, efi_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t get_next_variable_name_handler(void *context, struct call_req* req)
{
	struct smm_variable_provider *this_instance = (struct smm_variable_provider*)context;

	size_t param_len = 0;
	efi_status_t efi_status = sanitize_get_next_var_name_param(req, &param_len);

	if (efi_status == EFI_SUCCESS) {

		/* Valid get next variable name header */
		struct call_param_buf *resp_buf = call_req_get_resp_buf(req);

		if (resp_buf->size >= param_len) {

			struct call_param_buf *req_buf = call_req_get_req_buf(req);
			size_t max_name_len = resp_buf->size -
				SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_NAME_OFFSET;

			memmove(resp_buf->data, req_buf->data, param_len);

			efi_status = uefi_variable_store_get_next_variable_name(
				&this_instance->variable_store,
				(SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME*)resp_buf->data,
				max_name_len,
				&resp_buf->data_len);
		}
		else {

			/* Reponse buffer not big enough */
			efi_status = EFI_BAD_BUFFER_SIZE;
		}
	}

	call_req_set_opstatus(req, efi_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t set_variable_handler(void *context, struct call_req* req)
{
	struct smm_variable_provider *this_instance = (struct smm_variable_provider*)context;

	size_t param_len = 0;
	efi_status_t efi_status = sanitize_access_variable_param(req, &param_len);

	if (efi_status == EFI_SUCCESS) {

		/* Access variable header is whole.  Check that buffer length can
		 * accommodate the data.
		 */
		struct call_param_buf *req_buf = call_req_get_req_buf(req);

		const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *access_var =
			(const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE*)req_buf->data;

		size_t space_for_data = req_buf->data_len -
			SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_DATA_OFFSET(access_var);

		if (access_var->DataSize <= space_for_data) {

			efi_status = uefi_variable_store_set_variable(
				&this_instance->variable_store,
				access_var);
		}
		else {

			/* Invalid DataSize */
			efi_status = EFI_INVALID_PARAMETER;
		}
	}

	call_req_set_opstatus(req, efi_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t query_variable_info_handler(void *context, struct call_req* req)
{
	struct smm_variable_provider *this_instance = (struct smm_variable_provider*)context;

	/* todo */

	return TS_RPC_ERROR_NOT_READY;
}

static rpc_status_t exit_boot_service_handler(void *context, struct call_req* req)
{
	struct smm_variable_provider *this_instance = (struct smm_variable_provider*)context;

	efi_status_t efi_status = uefi_variable_store_exit_boot_service(&this_instance->variable_store);
	call_req_set_opstatus(req, efi_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t set_var_check_property_handler(void *context, struct call_req *req)
{
	struct smm_variable_provider *this_instance = (struct smm_variable_provider*)context;

	size_t param_len = 0;
	efi_status_t efi_status = sanitize_var_check_property_param(req, &param_len);

	if (efi_status == EFI_SUCCESS) {

		/* Request parameter structue looks whole */
		struct call_param_buf *req_buf = call_req_get_req_buf(req);

		const SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY *check_property =
			(const SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY*)req_buf->data;

		efi_status = uefi_variable_store_set_var_check_property(
			&this_instance->variable_store,
			check_property);
	}

	call_req_set_opstatus(req, efi_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t get_var_check_property_handler(void *context, struct call_req *req)
{
	struct smm_variable_provider *this_instance = (struct smm_variable_provider*)context;

	size_t param_len = 0;
	efi_status_t efi_status = sanitize_var_check_property_param(req, &param_len);

	if (efi_status == EFI_SUCCESS) {

		/* Request parameter structue looks whole */
		struct call_param_buf *resp_buf = call_req_get_resp_buf(req);

		if (resp_buf->size >= param_len) {

			struct call_param_buf *req_buf = call_req_get_req_buf(req);
			memmove(resp_buf->data, req_buf->data, param_len);
			resp_buf->data_len = param_len;

			efi_status = uefi_variable_store_get_var_check_property(
				&this_instance->variable_store,
				(SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY*)resp_buf->data);
		}
		else {

			/* Reponse buffer not big enough */
			efi_status = EFI_BAD_BUFFER_SIZE;
		}
	}

	call_req_set_opstatus(req, efi_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t get_payload_size_handler(void *context, struct call_req *req)
{
	struct smm_variable_provider *this_instance = (struct smm_variable_provider*)context;

	/* Payload size is constrained by the size of the RPC call buffer.  Because the variable length
	 * name is also carried in the buffer, the maximum payload size depends on the name size.  This
	 * function therefore treats the payload as name + data.
	 */
	size_t payload_size = req->req_buf.size - SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_NAME_OFFSET;

	efi_status_t efi_status = EFI_SUCCESS;
	struct call_param_buf *resp_buf = call_req_get_resp_buf(req);

	if (resp_buf->size >= sizeof(SMM_VARIABLE_COMMUNICATE_GET_PAYLOAD_SIZE)) {

		SMM_VARIABLE_COMMUNICATE_GET_PAYLOAD_SIZE *resp_msg =
			(SMM_VARIABLE_COMMUNICATE_GET_PAYLOAD_SIZE*)resp_buf->data;

		resp_msg->VariablePayloadSize = payload_size;
		resp_buf->data_len = sizeof(SMM_VARIABLE_COMMUNICATE_GET_PAYLOAD_SIZE);
	}
	else {

		/* Reponse buffer not big enough */
		efi_status = EFI_BAD_BUFFER_SIZE;
	}

	call_req_set_opstatus(req, efi_status);

	return TS_RPC_CALL_ACCEPTED;
}
