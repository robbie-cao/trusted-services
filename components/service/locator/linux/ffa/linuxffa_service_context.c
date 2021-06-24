/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "linuxffa_service_context.h"
#include <rpc/ffarpc/caller/linux/ffarpc_caller.h>
#include <stdlib.h>

/* Concrete service_context methods */
static rpc_session_handle linuxffa_service_context_open(void *context, struct rpc_caller **caller);
static void linuxffa_service_context_close(void *context, rpc_session_handle session_handle);
static void linuxffa_service_context_relinquish(void *context);


struct linuxffa_service_context *linuxffa_service_context_create(const char *dev_path,
    uint16_t partition_id, uint16_t iface_id)
{
    struct linuxffa_service_context *new_context =
        (struct linuxffa_service_context*)malloc(sizeof(struct linuxffa_service_context));

    if (new_context) {
        new_context->ffa_dev_path = dev_path;
        new_context->partition_id = partition_id;
        new_context->iface_id = iface_id;

        new_context->service_context.context = new_context;
        new_context->service_context.open = linuxffa_service_context_open;
        new_context->service_context.close = linuxffa_service_context_close;
        new_context->service_context.relinquish = linuxffa_service_context_relinquish;
    }

    return new_context;
}

static rpc_session_handle linuxffa_service_context_open(void *context, struct rpc_caller **caller)
{
    struct linuxffa_service_context *this_context = (struct linuxffa_service_context*)context;
    rpc_session_handle session_handle = NULL;
    struct ffarpc_caller *ffarpc_caller =
        (struct ffarpc_caller*)malloc(sizeof(struct ffarpc_caller));

    if (ffarpc_caller) {

        int status;
        *caller = ffarpc_caller_init(ffarpc_caller, this_context->ffa_dev_path);
        status = ffarpc_caller_open(ffarpc_caller,
            this_context->partition_id, this_context->iface_id);

		if (status == 0) {
            /* Successfully opened session */
            session_handle = ffarpc_caller;
		}
		else {
            /* Failed to open session */
			ffarpc_caller_close(ffarpc_caller);
            ffarpc_caller_deinit(ffarpc_caller);
            free(ffarpc_caller);
		}
    }

    return session_handle;
}

static void linuxffa_service_context_close(void *context, rpc_session_handle session_handle)
{
    struct ffarpc_caller *ffarpc_caller = (struct ffarpc_caller*)session_handle;

    (void)context;

    if (ffarpc_caller) {

		ffarpc_caller_close(ffarpc_caller);
        ffarpc_caller_deinit(ffarpc_caller);
        free(ffarpc_caller);
    }
}

static void linuxffa_service_context_relinquish(void *context)
{
    struct linuxffa_service_context *this_context = (struct linuxffa_service_context*)context;
    free(this_context);
}
