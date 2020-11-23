/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DUMMY_CALLER
#define DUMMY_CALLER

#include <rpc_caller.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * An rpc_caller that is used to return a suitable permanent status
 * code if an attempt is made to invoke a remote method where an
 * end-to-end rpc session has failed to be established.  Intended
 * to be used when a session with a real rpc endpoint cant't be
 * established but a client doesn't wish to treat the condition
 * as a fatal error.
 */
struct dummy_caller
{
    struct rpc_caller rpc_caller;
    rpc_status_t rpc_status;
    int opstatus;
    uint8_t *req_buf;
};

struct rpc_caller *dummy_caller_init(struct dummy_caller *s,
                                    rpc_status_t rpc_status, int opstatus);
void dummy_caller_deinit(struct dummy_caller *s);

#ifdef __cplusplus
}
#endif

#endif /* DUMMY_CALLER */
