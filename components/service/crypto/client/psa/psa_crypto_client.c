/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include "psa_crypto_client.h"

/* The singleton psa_crypto_client state */
struct psa_crypto_client psa_crypto_client_instance;

psa_status_t psa_crypto_client_init(struct rpc_caller *caller)
{
    psa_crypto_client_instance.caller = caller;
    return PSA_SUCCESS;
}

void psa_crypto_client_deinit(void)
{
    psa_crypto_client_instance.caller = NULL;
}

int psa_crypto_client_rpc_status(void)
{
    return psa_crypto_client_instance.rpc_status;
}
