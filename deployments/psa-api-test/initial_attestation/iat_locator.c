/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <psa/crypto.h>
#include <service_locator.h>
#include <service/attestation/client/psa/iat_client.h>
#include <service/attestation/client/provision/attest_provision_client.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include "../service_under_test.h"

/* RPC context */
static struct rpc_caller_session *session = NULL;
static struct service_context *attestation_service_context = NULL;

int locate_service_under_test(void)
{
	int status = -1;

	/* Attestation tests depend on PSA crypto so ensure library is initialised */
	psa_status_t psa_status = psa_crypto_init();

	if ((psa_status == PSA_SUCCESS) && !session && !attestation_service_context) {

		service_locator_init();

		attestation_service_context =
			service_locator_query("sn:trustedfirmware.org:attestation:0");

		if (attestation_service_context) {

			session = service_context_open(attestation_service_context);

			if (session) {

				psa_iat_client_init(session);
				attest_provision_client_init(session);

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

int relinquish_service_under_test(void)
{
	psa_iat_client_deinit();
	attest_provision_client_deinit();

	if (attestation_service_context && session) {

		service_context_close(attestation_service_context, session);
		session = NULL;
	}

	if (attestation_service_context) {

		service_context_relinquish(attestation_service_context);
		attestation_service_context = NULL;
	}

	return 0;
}
