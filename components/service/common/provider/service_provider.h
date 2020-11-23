/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SERVICE_PROVIDER_H
#define SERVICE_PROVIDER_H

#include <rpc/common/endpoint/call_ep.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Service handler
 *
 * Defines a mapping between an opcode and a handler function. A complete
 * service interface is defined by an array of service request handlers.
 */
struct service_handler {
	uint32_t opcode;
	rpc_status_t (*invoke)(void *context, struct call_req* req);
};

static inline int service_handler_invoke(const struct service_handler *handler,
						  void *context, struct call_req* req)
{
	return handler->invoke(context, req);
}

static inline uint32_t service_handler_get_opcode(const struct service_handler *handler)
{
	return handler->opcode;
}

/** \brief Service provider
 *
 * A generalised service provider that acts as an rpc call endpoint.  It receives call
 * requests and delegates them to the approprate handle provided by a concrete service
 * provider.
 */
struct service_provider {
    struct call_ep base;
    const struct service_handler *handlers;
    size_t num_handlers;
    call_param_serializer_ptr default_serializer;
};

static inline struct call_ep *service_provider_get_call_ep(struct service_provider *sp)
{
	return &sp->base;
}

void service_provider_init(struct service_provider *sp, void *context,
			     	const struct service_handler *handlers,
			     	size_t num_handlers);

static inline void service_set_default_serializer(struct service_provider *sp,
    				call_param_serializer_ptr serializer)
{
    sp->default_serializer = serializer;
}

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_PROVIDER_H */
