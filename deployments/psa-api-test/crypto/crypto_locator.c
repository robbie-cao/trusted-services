/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <service_locator.h>
#include <service/crypto/client/psa/psa_crypto_client.h>
#include <service/discovery/client/discovery_client.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include "../service_under_test.h"

/* RPC context */
static rpc_session_handle session_handle = NULL;
static struct service_context *crypto_service_context = NULL;

int locate_service_under_test(struct logging_caller *call_logger)
{
	int status = -1;

	if (!session_handle && !crypto_service_context) {

		struct rpc_caller *caller;

		crypto_service_context =
			service_locator_query("sn:trustedfirmware.org:crypto:0", &status);

		if (crypto_service_context) {

			session_handle =
				service_context_open(crypto_service_context, TS_RPC_ENCODING_PACKED_C, &caller);

			if (session_handle) {

				if (call_logger) {

					psa_crypto_client_init(logging_caller_attach(call_logger, caller));
				}
				else {

					psa_crypto_client_init(caller);
				}

				discovery_client_get_service_info(psa_crypto_client_base());

				status = 0;
			}
			else {

				status = -1;
				relinquish_service_under_test();
			}
		}
	}

	return status;
}

void relinquish_service_under_test(void)
{
	psa_crypto_client_deinit();

	if (crypto_service_context && session_handle) {

		service_context_close(crypto_service_context, session_handle);
		session_handle = NULL;
	}

	if (crypto_service_context) {

		service_context_relinquish(crypto_service_context);
		crypto_service_context = NULL;
	}
}
