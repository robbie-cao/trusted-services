/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PSA_CRYPTO_CLIENT_H
#define PSA_CRYPTO_CLIENT_H

#include <psa/error.h>
#include <rpc_caller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      The singleton psa_crypto_client state
 *
 * The psa crypto C API assumes a single instance of the backend provider.  This
 * structure defines the state used by the psa_cryypto_client that communicates
 * with a remote provider using the provided rpc caller.
 */
struct psa_crypto_client
{
    struct rpc_caller *caller;
    int rpc_status;
    psa_status_t init_status;
};

extern struct psa_crypto_client psa_crypto_client_instance;

/**
 * @brief      Initialises the single psa crypto client
 *
 * Assignes a concrete rpc caller to the psa crypto client and performs any other
 * initialisation needed.
 *
 * @param[in]  caller An initailised rpc_caller
 *
 * @return     A status indicating the success/failure of the operation
 */
psa_status_t psa_crypto_client_init(struct rpc_caller *caller);

/**
 * @brief      De-initialises the single psa crypto client
 *
 * Performs clean-up when the crypto client is no longer needed
 */
void psa_crypto_client_deinit(void);

/**
 * @brief      Get most recent rpc status
 *
 * Returns the error status for the most recent RPC operation
 *
 * @return     RPC status code
 */
int psa_crypto_client_rpc_status(void);

#ifdef __cplusplus
}
#endif

#endif /* PSA_CRYPTO_CLIENT_H */
