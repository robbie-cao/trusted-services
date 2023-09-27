/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <service_locator.h>
#include <service/secure_storage/frontend/psa/its/its_frontend.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include "../service_under_test.h"

/* RPC context */
static struct rpc_caller_session *session = NULL;
static struct service_context *ps_service_context = NULL;
static struct secure_storage_client storage_client;

int locate_service_under_test(void)
{
	int status = -1;

	service_locator_init();

	if (!session && !ps_service_context) {

		ps_service_context =
			service_locator_query("sn:trustedfirmware.org:internal-trusted-storage:0");

		if (ps_service_context) {

			session =
				service_context_open(ps_service_context);

			if (session) {

				struct storage_backend *storage_backend = NULL;
				status = -1;

				storage_backend = secure_storage_client_init(&storage_client, session);

				if (storage_backend) {

					psa_its_frontend_init(storage_backend);
					status = 0;
				}
			}

			if (status < 0) relinquish_service_under_test();
		}
	}

	return status;
}

int relinquish_service_under_test(void)
{
	psa_its_frontend_init(NULL);
	secure_storage_client_deinit(&storage_client);

	if (ps_service_context && session) {

		service_context_close(ps_service_context, session);
		session = NULL;
	}

	if (ps_service_context) {

		service_context_relinquish(ps_service_context);
		ps_service_context = NULL;
	}

	return 0;
}
