/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "dummy_caller.h"
#include <stdlib.h>

static rpc_call_handle call_begin(void *context, uint8_t **req_buf, size_t req_len);
static rpc_status_t call_invoke(void *context, rpc_call_handle handle, uint32_t opcode,
		     	int *opstatus, uint8_t **resp_buf, size_t *resp_len);
static void call_end(void *context, rpc_call_handle handle);


struct rpc_caller *dummy_caller_init(struct dummy_caller *s,
    rpc_status_t rpc_status, int opstatus)
{
    struct rpc_caller *base = &s->rpc_caller;

    rpc_caller_init(base, s);
    base->call_begin = call_begin;
    base->call_invoke = call_invoke;
    base->call_end = call_end;

    s->rpc_status = rpc_status;
    s->opstatus = opstatus;
    s->req_buf = NULL;

    return base;
}

void dummy_caller_deinit(struct dummy_caller *s)
{
    free(s->req_buf);
    s->req_buf = NULL;
}

static rpc_call_handle call_begin(void *context, uint8_t **req_buf, size_t req_len)
{
    struct dummy_caller *this_context = (struct dummy_caller*)context;
    rpc_call_handle handle = this_context;

    free(this_context->req_buf);
    this_context->req_buf = malloc(req_len);
    *req_buf = this_context->req_buf;

    return handle;
}

static rpc_status_t call_invoke(void *context, rpc_call_handle handle, uint32_t opcode,
		     	int *opstatus, uint8_t **resp_buf, size_t *resp_len)
{
    struct dummy_caller *this_context = (struct dummy_caller*)context;

    free(this_context->req_buf);
    this_context->req_buf = NULL;

    *resp_buf = NULL;
    *resp_len = 0;
    *opstatus = this_context->opstatus;

    return this_context->rpc_status;
}

static void call_end(void *context, rpc_call_handle handle)
{
    (void)context;
    (void)handle;
}
