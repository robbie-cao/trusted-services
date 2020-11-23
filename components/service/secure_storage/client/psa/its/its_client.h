/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PSA_ITS_CLIENT_H
#define PSA_ITS_CLIENT_H

#include <psa/error.h>
#include <rpc_caller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      Assignes a concrete rpc caller to the ITS library and initialises
 *             the library state.
 *
 * @param[in]  rpc_caller RPC caller instance
 *
 * @return     A status indicating the success/failure of the operation
 */
psa_status_t psa_its_client_init(struct rpc_caller *caller);

#ifdef __cplusplus
}
#endif

#endif /* PSA_ITS_CLIENT_H */
