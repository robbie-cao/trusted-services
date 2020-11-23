/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "service_provider.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <stddef.h>

static const struct service_handler *find_handler(const struct service_provider *sp,
						      uint32_t opcode)
{
	const struct service_handler *handler = NULL;
	size_t index = 0;

	if (sp->num_handlers) {
		while (index < sp->num_handlers) {
			if (service_handler_get_opcode(&sp->handlers[index]) == opcode) {
				handler = &sp->handlers[index];
				break;
			}
			++index;
		}
	}

	return handler;
}

static rpc_status_t receive(struct call_ep *base_ep, struct call_req *req)
{
	rpc_status_t rpc_status;
	struct service_provider *sp = NULL;
	const struct service_handler *handler = NULL;

	sp = (struct service_provider*)((char*)base_ep - offsetof(struct service_provider, base));
	handler = find_handler(sp, call_req_get_opcode(req));

    if (handler) {

        req->serializer = sp->default_serializer;
        rpc_status = service_handler_invoke(handler, base_ep->context, req);
    }
    else {

        rpc_status = TS_RPC_ERROR_INVALID_OPCODE;
    }

	return rpc_status;
}

void service_provider_init(struct service_provider *sp, void *context,
			     const struct service_handler *handlers,
			     size_t num_handlers)
{
	sp->base.receive = receive;
	sp->base.context = context;

	sp->handlers = handlers;
	sp->num_handlers = num_handlers;

	sp->default_serializer = NULL;
}
