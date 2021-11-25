/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "direct_caller.h"
#include <rpc/common/endpoint/rpc_interface.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <stdlib.h>

#define DIRECT_CALLER_DEFAULT_REQ_BUF_SIZE      (4096)
#define DIRECT_CALLER_DEFAULT_RESP_BUF_SIZE     (4096)

static rpc_call_handle call_begin(void *context, uint8_t **req_buf, size_t req_len);
static rpc_status_t call_invoke(void *context, rpc_call_handle handle, uint32_t opcode,
		     	rpc_opstatus_t *opstatus, uint8_t **resp_buf, size_t *resp_len);
static void call_end(void *context, rpc_call_handle handle);


struct rpc_caller *direct_caller_init(struct direct_caller *s, struct rpc_interface *iface,
                        size_t req_buf_size, size_t resp_buf_size)
{
    struct rpc_caller *base = &s->rpc_caller;

    rpc_caller_init(base, s);
    base->call_begin = call_begin;
    base->call_invoke = call_invoke;
    base->call_end = call_end;

    s->rpc_interface = iface;
    s->caller_id = 0;
    s->is_call_transaction_in_progess = false;
    s->req_len = 0;
    s->req_buf_size = req_buf_size;
    s->resp_buf_size = resp_buf_size;
    s->req_buf = malloc(s->req_buf_size);
    s->resp_buf = malloc(s->resp_buf_size);

    if (!s->req_buf || !s->resp_buf) {

        /* Buffer allocation failed */
        base = NULL;
        direct_caller_deinit(s);
    }

    return base;
}

struct rpc_caller *direct_caller_init_default(struct direct_caller *s, struct rpc_interface *iface)
{
    /* Initialise with default buffer sizes */
    return direct_caller_init(s, iface,
        DIRECT_CALLER_DEFAULT_REQ_BUF_SIZE,
        DIRECT_CALLER_DEFAULT_RESP_BUF_SIZE);
}

void direct_caller_deinit(struct direct_caller *s)
{
    free(s->req_buf);
    s->req_buf = NULL;
    free(s->resp_buf);
    s->resp_buf = NULL;
}

static rpc_call_handle call_begin(void *context, uint8_t **req_buf, size_t req_len)
{
    struct direct_caller *this_context = (struct direct_caller*)context;
    rpc_call_handle handle = NULL;

    if (!this_context->is_call_transaction_in_progess &&
        (req_len <= this_context->req_buf_size)) {

        this_context->is_call_transaction_in_progess = true;

        if (req_buf){
            *req_buf = this_context->req_buf;
            this_context->req_len = req_len;
        }

        handle = this_context;
    }

    return handle;
}

static rpc_status_t call_invoke(void *context, rpc_call_handle handle, uint32_t opcode,
		     	rpc_opstatus_t *opstatus, uint8_t **resp_buf, size_t *resp_len)
{
    struct direct_caller *this_context = (struct direct_caller*)context;
    rpc_status_t status = TS_RPC_ERROR_INVALID_TRANSACTION;

    if ((handle == this_context) && this_context->is_call_transaction_in_progess) {

        struct call_req req;

        req.interface_id = 0;
        req.opcode = opcode;
        req.encoding = this_context->rpc_caller.encoding;
        req.caller_id = this_context->caller_id;
        req.opstatus = 0;
        req.req_buf = call_param_buf_init_full(this_context->req_buf,
                            this_context->req_buf_size, this_context->req_len);
        req.resp_buf = call_param_buf_init_empty(this_context->resp_buf,
                            this_context->resp_buf_size);

        status = rpc_interface_receive(this_context->rpc_interface, &req);

        *resp_buf = this_context->resp_buf;
        *resp_len = call_req_get_resp_buf(&req)->data_len;
        *opstatus = call_req_get_opstatus(&req);
    }

    return status;
}

static void call_end(void *context, rpc_call_handle handle)
{
    struct direct_caller *this_context = (struct direct_caller*)context;

    if ((handle == this_context) && this_context->is_call_transaction_in_progess) {

        this_context->req_len = 0;
        this_context->is_call_transaction_in_progess = false;
    }
}
