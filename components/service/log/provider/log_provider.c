// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include "log_provider.h"
#include "components/rpc/common/endpoint/rpc_interface.h"
#include "components/service/log/common/log_status.h"
#include "protocols/rpc/common/packed-c/status.h"
#include "protocols/service/log/packed-c/log_proto.h"
#include "util.h"

static rpc_status_t log_puts_handler(void *context, struct call_req *req);

static const struct service_handler handler_table[] = { { TS_LOG_OPCODE_PUTS, log_puts_handler } };

struct rpc_interface *log_provider_init(struct log_provider *context,
					struct log_interface *interface)
{
	struct rpc_interface *rpc_interface = NULL;

	if (context == NULL || interface == NULL)
		goto out;

	service_provider_init(&context->base_provider, context, handler_table,
			      ARRAY_SIZE(handler_table));

	context->interface = interface;

	rpc_interface = service_provider_get_rpc_interface(&context->base_provider);

out:
	return rpc_interface;
}

static rpc_status_t log_puts_handler(void *context, struct call_req *req)
{
	struct log_request *request_desc;
	size_t request_data_length = 0;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct log_request))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct log_request *)(req->req_buf.data);

	/* Checking for overflow */
	if (ADD_OVERFLOW(sizeof(*request_desc), request_desc->msg_length, &request_data_length))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	/* Checking if descriptor and data fits into the request buffer */
	if (req->req_buf.data_len < request_data_length)
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	/* Make sure it is null terminated */
	request_desc->msg[request_desc->msg_length] = '\0';

	((struct log_provider *)context)->interface->puts(request_desc->msg);
	call_req_set_opstatus(req, LOG_STATUS_SUCCESS);

	return TS_RPC_CALL_ACCEPTED;
}
