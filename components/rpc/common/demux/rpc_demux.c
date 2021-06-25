/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <protocols/rpc/common/packed-c/status.h>
#include "rpc_demux.h"

static rpc_status_t receive(struct rpc_interface *rpc_iface, struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERFACE_DOES_NOT_EXIST;
	struct rpc_demux *context = (struct rpc_demux*)rpc_iface->context;

	unsigned int iface_id = call_req_get_interface_id(req);

	if ((iface_id < RPC_DEMUX_MAX_OUTPUTS) && context->outputs[iface_id]) {

		rpc_status = rpc_interface_receive(context->outputs[iface_id], req);
	}

	return rpc_status;
}

struct rpc_interface *rpc_demux_init(struct rpc_demux *context)
{
	context->input.receive = receive;
	context->input.context = context;

	for (unsigned int i = 0; i < RPC_DEMUX_MAX_OUTPUTS; ++i) {

		context->outputs[i] = NULL;
	}

	return &context->input;
}

void rpc_demux_deinit(struct rpc_demux *context)
{
	(void)context;
}

void rpc_demux_attach(struct rpc_demux *context,
	unsigned int iface_id, struct rpc_interface *output)
{
	assert(iface_id < RPC_DEMUX_MAX_OUTPUTS);
	context->outputs[iface_id] = output;
}

void rpc_demux_dettach(struct rpc_demux *context,
	unsigned int iface_id)
{
	assert(iface_id < RPC_DEMUX_MAX_OUTPUTS);
	context->outputs[iface_id] = NULL;
}
