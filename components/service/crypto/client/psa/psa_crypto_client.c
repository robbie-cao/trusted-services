/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include "psa_crypto_client.h"

/* The singleton psa_crypto_client state */
struct psa_crypto_client psa_crypto_client_instance = {

	.caller = NULL,

	/* To conform to PSA API, psa_crypto_init needs to be called.
	 * This state variable is used enforces this.
	 */
	.init_status = PSA_ERROR_BAD_STATE
};

psa_status_t psa_crypto_init(void) {

	/* Must be called after psa_crypto_client_init */
	if (psa_crypto_client_instance.caller) {

		psa_crypto_client_instance.init_status = PSA_SUCCESS;
	}

	return psa_crypto_client_instance.init_status;
}

psa_status_t psa_crypto_client_init(struct rpc_caller *caller)
{
	psa_crypto_client_instance.caller = caller;
	return PSA_SUCCESS;
}

void psa_crypto_client_deinit(void)
{
	psa_crypto_client_instance.caller = NULL;
	psa_crypto_client_instance.init_status = PSA_ERROR_BAD_STATE;
}

int psa_crypto_client_rpc_status(void)
{
	return psa_crypto_client_instance.rpc_status;
}
