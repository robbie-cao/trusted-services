/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STANDALONE_SMM_VARIABLE_SERVICE_CONTEXT_H
#define STANDALONE_SMM_VARIABLE_SERVICE_CONTEXT_H

#include <stddef.h>
#include <stdint.h>
#include <service/locator/standalone/standalone_service_context.h>
#include <service/smm_variable/provider/smm_variable_provider.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>
#include <service/secure_storage/backend/mock_store/mock_store.h>

class smm_variable_service_context : public standalone_service_context
{
public:
	smm_variable_service_context(const char *sn);
	virtual ~smm_variable_service_context();

private:

	void do_init();
	void do_deinit();

	static const size_t MAX_VARIABLES = 40;

	struct smm_variable_provider m_smm_variable_provider;
	struct secure_storage_client m_persistent_store_client;
	struct mock_store m_volatile_store;
	struct service_context *m_storage_service_context;
	rpc_session_handle m_storage_session_handle;
};

#endif /* STANDALONE_SMM_VARIABLE_SERVICE_CONTEXT_H */
