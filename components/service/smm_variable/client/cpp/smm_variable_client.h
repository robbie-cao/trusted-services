/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMM_VARIABLE_CLIENT_H
#define SMM_VARIABLE_CLIENT_H

#include <cstdint>
#include <string>
#include <vector>
#include <protocols/common/efi/efi_status.h>
#include <protocols/service/smm_variable/smm_variable_proto.h>

struct rpc_caller;

/*
 * Provides a C++ client interface for accessing an instance of the smm-variable service.
 * This client is intented for testing the UEFI variable store provided by the smm-variable
 * service.
 */
class smm_variable_client
{
public:

	smm_variable_client();
	smm_variable_client(struct rpc_caller *caller);
	~smm_variable_client();

	void set_caller(struct rpc_caller *caller);
	int err_rpc_status() const;

	/* Set a string type variable */
	efi_status_t set_variable(
		const EFI_GUID &guid,
		const std::wstring &name,
		const std::string &data,
		uint32_t attributes);

	efi_status_t set_variable(
		const EFI_GUID &guid,
		const std::wstring &name,
		const std::string &data,
		uint32_t attributes,
		size_t override_name_size,
		size_t override_data_size);

	/* Get a string type variable */
	efi_status_t get_variable(
		const EFI_GUID &guid,
		const std::wstring &name,
		std::string &data);

	efi_status_t get_variable(
		const EFI_GUID &guid,
		const std::wstring &name,
		std::string &data,
		size_t override_name_size);

	/* Remove a variable */
	efi_status_t remove_variable(
		const EFI_GUID &guid,
		const std::wstring &name);

	/* Query variable info */
	efi_status_t query_variable_info(
		uint32_t attributes,
		size_t *max_variable_storage_size,
		size_t *remaining_variable_storage_size,
		size_t *max_variable_size);

	/* Get the next variable name - for enumerating store contents */
	efi_status_t get_next_variable_name(
		EFI_GUID &guid,
		std::wstring &name);

	efi_status_t get_next_variable_name(
		EFI_GUID &guid,
		std::wstring &name,
		size_t override_name_size);

	/* Exit boot service */
	efi_status_t exit_boot_service();

	/* Set variable check properties */
	efi_status_t set_var_check_property(
		const EFI_GUID &guid,
		const std::wstring &name,
		const VAR_CHECK_VARIABLE_PROPERTY &check_property);

	efi_status_t set_var_check_property(
		const EFI_GUID &guid,
		const std::wstring &name,
		const VAR_CHECK_VARIABLE_PROPERTY &check_property,
		size_t override_name_size);

	/* Get variable check properties */
	efi_status_t get_var_check_property(
		const EFI_GUID &guid,
		const std::wstring &name,
		VAR_CHECK_VARIABLE_PROPERTY &check_property);

	efi_status_t get_var_check_property(
		const EFI_GUID &guid,
		const std::wstring &name,
		VAR_CHECK_VARIABLE_PROPERTY &check_property,
		size_t override_name_size);

	/* Get maximum variable payload size */
	efi_status_t get_payload_size(
		size_t &payload_size);


private:
	efi_status_t rpc_to_efi_status() const;

	static std::vector<int16_t> to_variable_name(const std::wstring &string);
	static const std::wstring from_variable_name(const int16_t *name, size_t name_size);

	struct rpc_caller *m_caller;
	int m_err_rpc_status;
};

#endif /* SMM_VARIABLE_CLIENT_H */
