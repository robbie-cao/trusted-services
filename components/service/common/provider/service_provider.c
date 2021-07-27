/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
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

	if ((opcode >= sp->opcode_range_lo) && (opcode <= sp->opcode_range_hi)) {

		size_t index = 0;

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

static void set_opcode_range(struct service_provider *sp)
{
	uint32_t lo = UINT32_MAX;
	uint32_t hi = 0;

	/* Determine opcode range so that this service may be skipped
	 * without having to iterate over all handlers.  This reduces
	 * the time to find a qualifying handler when multiple service
	 * providers are chained.
	 */
	for (size_t index = 0; index < sp->num_handlers; index++) {

		uint32_t opcode = service_handler_get_opcode(&sp->handlers[index]);

		if (opcode < lo) lo = opcode;
		if (opcode > hi) hi = opcode;
	}

	sp->opcode_range_lo = lo;
	sp->opcode_range_hi = hi;
}

static rpc_status_t receive(struct rpc_interface *rpc_iface, struct call_req *req)
{
	rpc_status_t rpc_status;
	struct service_provider *sp = NULL;
	const struct service_handler *handler = NULL;

	sp = (struct service_provider*)((char*)rpc_iface - offsetof(struct service_provider, iface));
	handler = find_handler(sp, call_req_get_opcode(req));

	if (handler) {

		 rpc_status = service_handler_invoke(handler, rpc_iface->context, req);
	}
	else if (sp->successor) {

		rpc_status = rpc_interface_receive(sp->successor, req);
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
	sp->iface.receive = receive;
	sp->iface.context = context;

	sp->handlers = handlers;
	sp->num_handlers = num_handlers;

	sp->successor = NULL;

	set_opcode_range(sp);
}

void service_provider_extend(struct service_provider *context,
                    struct service_provider *sub_provider)
{
	sub_provider->successor = context->successor;
	context->successor = &sub_provider->iface;
}
