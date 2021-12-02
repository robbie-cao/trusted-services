/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstring>
#include <protocols/rpc/common/packed-c/status.h>
#include <rpc_caller.h>
#include "smm_variable_client.h"

smm_variable_client::smm_variable_client() :
	m_caller(NULL),
	m_err_rpc_status(TS_RPC_CALL_ACCEPTED)
{

}

smm_variable_client::smm_variable_client(
	struct rpc_caller *caller) :
	m_caller(caller),
	m_err_rpc_status(TS_RPC_CALL_ACCEPTED)
{

}

smm_variable_client::~smm_variable_client()
{

}

void smm_variable_client::set_caller(struct rpc_caller *caller)
{
	m_caller = caller;
}

int smm_variable_client::err_rpc_status() const
{
	return m_err_rpc_status;
}

efi_status_t smm_variable_client::set_variable(
	const EFI_GUID &guid,
	const std::wstring &name,
	const std::string &data,
	uint32_t attributes)
{
	return set_variable(
		guid,
		name,
		data,
		attributes,
		0, 0);
}

efi_status_t smm_variable_client::set_variable(
	const EFI_GUID &guid,
	const std::wstring &name,
	const std::string &data,
	uint32_t attributes,
	size_t override_name_size,
	size_t override_data_size)
{
	efi_status_t efi_status = EFI_NOT_READY;

	std::vector<int16_t> var_name = to_variable_name(name);
	size_t name_size = var_name.size() * sizeof(int16_t);
	size_t data_size = data.size();
	size_t req_len = SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_SIZE(name_size, data_size);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
        size_t resp_len;
		rpc_opstatus_t opstatus;

		SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *access_var =
			(SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE*)req_buf;

		access_var->Guid = guid;
		access_var->NameSize = name_size;
		access_var->DataSize = data_size;
		access_var->Attributes = attributes;

		memcpy(access_var->Name, var_name.data(), name_size);
		memcpy(&req_buf[SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_DATA_OFFSET(access_var)],
			data.data(), data_size);

		/* To support invalid size testing, use overrides if set */
		if (override_name_size) access_var->NameSize = override_name_size;
		if (override_data_size) access_var->DataSize = override_data_size;

		m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
			SMM_VARIABLE_FUNCTION_SET_VARIABLE, &opstatus, &resp_buf, &resp_len);

		if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

			efi_status = opstatus;
		}
		else {

			efi_status = rpc_to_efi_status();
		}

		rpc_caller_end(m_caller, call_handle);
	}

	return efi_status;
}

efi_status_t smm_variable_client::get_variable(
	const EFI_GUID &guid,
	const std::wstring &name,
	std::string &data)
{
	return get_variable(
		guid,
		name,
		data,
		0);
}

efi_status_t smm_variable_client::get_variable(
	const EFI_GUID &guid,
	const std::wstring &name,
	std::string &data,
	size_t override_name_size)
{
	efi_status_t efi_status = EFI_NOT_READY;

	std::vector<int16_t> var_name = to_variable_name(name);
	size_t name_size = var_name.size() * sizeof(int16_t);
	size_t data_size = 0;
	size_t req_len = SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_SIZE(name_size, data_size);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
        size_t resp_len;
		rpc_opstatus_t opstatus;

		SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *access_var =
			(SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE*)req_buf;

		access_var->Guid = guid;
		access_var->NameSize = name_size;
		access_var->DataSize = data_size;

		memcpy(access_var->Name, var_name.data(), name_size);

		/* To support invalid size testing, use overrides if set */
		if (override_name_size) access_var->NameSize = override_name_size;

		m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
			SMM_VARIABLE_FUNCTION_GET_VARIABLE, &opstatus, &resp_buf, &resp_len);

		if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

			efi_status = opstatus;

			if (efi_status == EFI_SUCCESS) {

				efi_status = EFI_PROTOCOL_ERROR;

				if (resp_len >= SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_NAME_OFFSET) {

					access_var = (SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE*)resp_buf;

					if (resp_len >=
						SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_TOTAL_SIZE(access_var)) {

						data_size = access_var->DataSize;
						const char *data_start = (const char*)
						&resp_buf[
							SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_DATA_OFFSET(access_var)];

						data.assign(data_start, data_size);
						efi_status = EFI_SUCCESS;
					}
				}
			}
		}
		else {

			efi_status = rpc_to_efi_status();
		}

		rpc_caller_end(m_caller, call_handle);
	}

	return efi_status;
}

efi_status_t smm_variable_client::remove_variable(
	const EFI_GUID &guid,
	const std::wstring &name)
{
	/* Variable is removed by performing a 'set' with zero length data */
	return set_variable(guid, name, std::string(), 0);
}

efi_status_t smm_variable_client::get_next_variable_name(
	EFI_GUID &guid,
	std::wstring &name)
{
	return get_next_variable_name(
		guid,
		name,
		0);
}

efi_status_t smm_variable_client::query_variable_info(
	uint32_t attributes,
	size_t *max_variable_storage_size,
	size_t *remaining_variable_storage_size,
	size_t *max_variable_size)
{
	efi_status_t efi_status = EFI_NOT_READY;

	size_t req_len = sizeof(SMM_VARIABLE_COMMUNICATE_QUERY_VARIABLE_INFO);
	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
        size_t resp_len;
		rpc_opstatus_t opstatus;

		SMM_VARIABLE_COMMUNICATE_QUERY_VARIABLE_INFO *query =
			(SMM_VARIABLE_COMMUNICATE_QUERY_VARIABLE_INFO*)req_buf;

		query->Attributes = attributes;
		query->MaximumVariableSize = 0;
		query->MaximumVariableStorageSize = 0;
		query->RemainingVariableStorageSize = 0;

		m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
			SMM_VARIABLE_FUNCTION_QUERY_VARIABLE_INFO, &opstatus, &resp_buf, &resp_len);

		if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

			efi_status = opstatus;

			if (efi_status == EFI_SUCCESS) {

				if (resp_len >= sizeof(SMM_VARIABLE_COMMUNICATE_QUERY_VARIABLE_INFO)) {

					query = (SMM_VARIABLE_COMMUNICATE_QUERY_VARIABLE_INFO*)resp_buf;

					*max_variable_storage_size = query->MaximumVariableStorageSize;
					*remaining_variable_storage_size = query->RemainingVariableStorageSize;
					*max_variable_size = query->MaximumVariableSize;
				}
				else {

					efi_status = EFI_PROTOCOL_ERROR;
				}
			}
			else {

				efi_status = EFI_PROTOCOL_ERROR;
			}
		}
		else {

			efi_status = rpc_to_efi_status();
		}

		rpc_caller_end(m_caller, call_handle);
	}

	return efi_status;
}

efi_status_t smm_variable_client::get_next_variable_name(
	EFI_GUID &guid,
	std::wstring &name,
	size_t override_name_size)
{
	efi_status_t efi_status = EFI_NOT_READY;

	std::vector<int16_t> var_name = to_variable_name(name);
	size_t name_size = var_name.size() * sizeof(int16_t);
	size_t req_len = SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_SIZE(name_size);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
        size_t resp_len;
		rpc_opstatus_t opstatus;

		SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME *next_var =
			(SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME*)req_buf;

		next_var->Guid = guid;
		next_var->NameSize = name_size;

		memcpy(next_var->Name, var_name.data(), name_size);

		/* To support invalid size testing, use overrides if set */
		if (override_name_size) next_var->NameSize = override_name_size;

		m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
			SMM_VARIABLE_FUNCTION_GET_NEXT_VARIABLE_NAME, &opstatus, &resp_buf, &resp_len);

		if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

			efi_status = opstatus;

			if (efi_status == EFI_SUCCESS) {

				efi_status = EFI_PROTOCOL_ERROR;

				if (resp_len >= SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_NAME_OFFSET) {

					next_var = (SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME*)resp_buf;

					if (resp_len >=
						SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_TOTAL_SIZE(next_var)) {

						guid = next_var->Guid;
						name = from_variable_name(next_var->Name, next_var->NameSize);

						efi_status = EFI_SUCCESS;
					}
				}
			}
		}
		else {

			efi_status = rpc_to_efi_status();
		}

		rpc_caller_end(m_caller, call_handle);
	}

	return efi_status;
}

efi_status_t smm_variable_client::exit_boot_service()
{
	efi_status_t efi_status = EFI_NOT_READY;

	size_t req_len = 0;
	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
        size_t resp_len;
		rpc_opstatus_t opstatus;

		m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
			SMM_VARIABLE_FUNCTION_EXIT_BOOT_SERVICE, &opstatus, &resp_buf, &resp_len);

		if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

			efi_status = opstatus;
		}
		else {

			efi_status = rpc_to_efi_status();
		}

		rpc_caller_end(m_caller, call_handle);
	}

	return efi_status;
}

efi_status_t smm_variable_client::set_var_check_property(
	const EFI_GUID &guid,
	const std::wstring &name,
	const VAR_CHECK_VARIABLE_PROPERTY &check_property)
{
	return set_var_check_property(
		guid,
		name,
		check_property,
		0);
}

efi_status_t smm_variable_client::set_var_check_property(
	const EFI_GUID &guid,
	const std::wstring &name,
	const VAR_CHECK_VARIABLE_PROPERTY &check_property,
	size_t override_name_size)
{
	efi_status_t efi_status = EFI_NOT_READY;

	std::vector<int16_t> var_name = to_variable_name(name);
	size_t name_size = var_name.size() * sizeof(int16_t);
	size_t req_len = SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY_SIZE(name_size);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
        size_t resp_len;
		rpc_opstatus_t opstatus;

		SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY *req_msg =
			(SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY*)req_buf;

		req_msg->Guid = guid;
		req_msg->NameSize = name_size;
		req_msg->VariableProperty = check_property;

		memcpy(req_msg->Name, var_name.data(), name_size);

		/* To support invalid size testing, use override if set */
		if (override_name_size) req_msg->NameSize = override_name_size;

		m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
			SMM_VARIABLE_FUNCTION_VAR_CHECK_VARIABLE_PROPERTY_SET, &opstatus,
			&resp_buf, &resp_len);

		if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

			efi_status = opstatus;
		}
		else {

			efi_status = rpc_to_efi_status();
		}

		rpc_caller_end(m_caller, call_handle);
	}

	return efi_status;
}

efi_status_t smm_variable_client::get_var_check_property(
	const EFI_GUID &guid,
	const std::wstring &name,
	VAR_CHECK_VARIABLE_PROPERTY &check_property)
{
	return get_var_check_property(
		guid,
		name,
		check_property,
		0);
}

efi_status_t smm_variable_client::get_var_check_property(
	const EFI_GUID &guid,
	const std::wstring &name,
	VAR_CHECK_VARIABLE_PROPERTY &check_property,
	size_t override_name_size)
{
	efi_status_t efi_status = EFI_NOT_READY;

	std::vector<int16_t> var_name = to_variable_name(name);
	size_t name_size = var_name.size() * sizeof(int16_t);
	size_t req_len = SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY_SIZE(name_size);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
        size_t resp_len;
		rpc_opstatus_t opstatus;

		SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY *req_msg =
			(SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY*)req_buf;

		req_msg->Guid = guid;
		req_msg->NameSize = name_size;

		memcpy(req_msg->Name, var_name.data(), name_size);

		/* To support invalid size testing, use overrides if set */
		if (override_name_size) req_msg->NameSize = override_name_size;

		m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
			SMM_VARIABLE_FUNCTION_VAR_CHECK_VARIABLE_PROPERTY_GET, &opstatus,
			&resp_buf, &resp_len);

		if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

			efi_status = opstatus;

			if (efi_status == EFI_SUCCESS) {

				efi_status = EFI_PROTOCOL_ERROR;

				if (resp_len >= SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY_NAME_OFFSET) {

					SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY *resp_msg =
						(SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY*)resp_buf;

					if (resp_len >=
						SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY_TOTAL_SIZE(resp_msg)) {

						check_property = resp_msg->VariableProperty;
						efi_status = EFI_SUCCESS;
					}
				}
			}
		}
		else {

			efi_status = rpc_to_efi_status();
		}

		rpc_caller_end(m_caller, call_handle);
	}

	return efi_status;
}

efi_status_t smm_variable_client::get_payload_size(
	size_t &payload_size)
{
	efi_status_t efi_status = EFI_NOT_READY;

	size_t req_len = 0;
	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

	if (call_handle) {

		uint8_t *resp_buf;
        size_t resp_len;
		rpc_opstatus_t opstatus;

		m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
			SMM_VARIABLE_FUNCTION_GET_PAYLOAD_SIZE, &opstatus, &resp_buf, &resp_len);

		if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

			efi_status = opstatus;

			if (efi_status == EFI_SUCCESS) {

				if (resp_len >= sizeof(SMM_VARIABLE_COMMUNICATE_GET_PAYLOAD_SIZE)) {

					SMM_VARIABLE_COMMUNICATE_GET_PAYLOAD_SIZE *resp_msg =
						(SMM_VARIABLE_COMMUNICATE_GET_PAYLOAD_SIZE*)resp_buf;

					payload_size = resp_msg->VariablePayloadSize;
				}
				else {

					efi_status = EFI_PROTOCOL_ERROR;
				}
			}
		}
		else {

			efi_status = rpc_to_efi_status();
		}

		rpc_caller_end(m_caller, call_handle);
	}

	return efi_status;
}

efi_status_t smm_variable_client::rpc_to_efi_status() const
{
	efi_status_t efi_status = EFI_INVALID_PARAMETER;

	switch (m_err_rpc_status)
	{
		case TS_RPC_ERROR_EP_DOES_NOT_EXIT:
			efi_status = EFI_UNSUPPORTED;
			break;
		case TS_RPC_ERROR_INVALID_OPCODE:
			efi_status = EFI_UNSUPPORTED;
			break;
		case TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED:
			efi_status = EFI_PROTOCOL_ERROR;
			break;
		case TS_RPC_ERROR_INVALID_REQ_BODY:
			efi_status = EFI_PROTOCOL_ERROR;
			break;
		case TS_RPC_ERROR_INVALID_RESP_BODY:
			efi_status = EFI_DEVICE_ERROR;
			break;
		case TS_RPC_ERROR_RESOURCE_FAILURE:
			efi_status = EFI_OUT_OF_RESOURCES;
			break;
		case TS_RPC_ERROR_NOT_READY:
			efi_status = EFI_NOT_READY;
			break;
		case TS_RPC_ERROR_INVALID_TRANSACTION:
			efi_status = EFI_INVALID_PARAMETER;
			break;
		case TS_RPC_ERROR_INTERNAL:
			efi_status = EFI_DEVICE_ERROR;
			break;
		case TS_RPC_ERROR_INVALID_PARAMETER:
			efi_status = EFI_INVALID_PARAMETER;
			break;
		case TS_RPC_ERROR_INTERFACE_DOES_NOT_EXIST:
			efi_status = EFI_UNSUPPORTED;
			break;
		default:
			break;
	}

	return efi_status;
}

std::vector<int16_t> smm_variable_client::to_variable_name(
	const std::wstring &string)
{
	std::vector<int16_t> var_name;

	for (size_t i = 0; i < string.size(); i++) {

		var_name.push_back((int16_t)string[i]);
	}

	var_name.push_back(0);

	return var_name;
}

const std::wstring smm_variable_client::from_variable_name(
	const int16_t *var_name,
	size_t name_size)
{
	std::wstring name;
	size_t num_chars = name_size / sizeof(int16_t);

	for (size_t i = 0; i < num_chars; i++) {

		if (!var_name[i])	break;  /* Reached null terminator */
		name.push_back((wchar_t)var_name[i]);
	}

	return name;
}
