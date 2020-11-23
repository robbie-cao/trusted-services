/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DIRECT_CALLER_H
#define DIRECT_CALLER_H

#include <rpc_caller.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct call_ep;

/** An rpc_caller that calls methods associated with a specific endpoint
 *  directly.  Used when the caller and endpoint are running in the same
 *  execution context.
 **/
struct direct_caller
{
    struct rpc_caller rpc_caller;
    struct call_ep *call_ep;
    uint32_t caller_id;
    bool is_call_transaction_in_progess;
    size_t req_len;
    size_t req_buf_size;
    size_t resp_buf_size;
    uint8_t *req_buf;
    uint8_t *resp_buf;
};

struct rpc_caller *direct_caller_init(struct direct_caller *s, struct call_ep *ep,
                        size_t req_buf_size, size_t resp_buf_size);

struct rpc_caller *direct_caller_init_default(struct direct_caller *s, struct call_ep *ep);

void direct_caller_deinit(struct direct_caller *s);

#ifdef __cplusplus
}
#endif

#endif /* DIRECT_CALLER_H */
