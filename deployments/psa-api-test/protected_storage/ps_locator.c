/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <service_locator.h>
#include <service/secure_storage/frontend/psa/ps/ps_frontend.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include "../service_under_test.h"

/* RPC context */
static rpc_session_handle session_handle = NULL;
static struct service_context *ps_service_context = NULL;
static struct secure_storage_client storage_client;

int locate_service_under_test(struct logging_caller *call_logger)
{
	int status = -1;

	if (!session_handle && !ps_service_context) {

		struct rpc_caller *caller;

		ps_service_context =
			service_locator_query("sn:trustedfirmware.org:protected-storage:0", &status);

		if (ps_service_context) {

			session_handle =
				service_context_open(ps_service_context, TS_RPC_ENCODING_PACKED_C, &caller);

			if (session_handle) {

				struct storage_backend *storage_backend = NULL;
				status = -1;

				if (call_logger) {

					storage_backend = secure_storage_client_init(&storage_client,
						logging_caller_attach(call_logger, caller));
				}
				else {

					storage_backend = secure_storage_client_init(&storage_client,  caller);
				}

				if (storage_backend) {

					psa_ps_frontend_init(storage_backend);
					status = 0;
				}
			}

			if (status < 0) relinquish_service_under_test();
		}
	}

	return status;
}

void relinquish_service_under_test(void)
{
	psa_ps_frontend_init(NULL);
	secure_storage_client_deinit(&storage_client);

	if (ps_service_context && session_handle) {

		service_context_close(ps_service_context, session_handle);
		session_handle = NULL;
	}

	if (ps_service_context) {

		service_context_relinquish(ps_service_context);
		ps_service_context = NULL;
	}
}
