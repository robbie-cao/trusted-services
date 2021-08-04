/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <psa/crypto.h>
#include <service/crypto/client/psa/psa_crypto_client.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <rpc/dummy/dummy_caller.h>
#include "stub_crypto_backend.h"

psa_status_t stub_crypto_backend_init(void)
{
	static struct dummy_caller dummy_caller;
	struct rpc_caller *caller = dummy_caller_init(&dummy_caller,
		TS_RPC_CALL_ACCEPTED, PSA_ERROR_SERVICE_FAILURE);

	psa_status_t status = psa_crypto_client_init(caller);

	if (status == PSA_SUCCESS)
		status = psa_crypto_init();

	return status;
}

void stub_crypto_backend_deinit(void)
{
	psa_crypto_client_deinit();
}
