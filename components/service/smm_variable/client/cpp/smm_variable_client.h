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

	/* Get a string type variable */
	efi_status_t get_variable(
		const EFI_GUID &guid,
		const std::wstring &name,
		std::string &data);

	/* Get the next variable name - for enumerating store contents */
	efi_status_t get_next_variable_name(
		EFI_GUID &guid,
		std::wstring &name);

	/* Exit boot service */
	efi_status_t exit_boot_service();

private:
	static std::vector<int16_t> to_variable_name(const std::wstring &string);
	static const std::wstring from_variable_name(const int16_t *name, size_t name_size);

	struct rpc_caller *m_caller;
	int m_err_rpc_status;
};

#endif /* SMM_VARIABLE_CLIENT_H */
