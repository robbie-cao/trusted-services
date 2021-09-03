// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include "log_client.h"
#include "protocols/service/log/packed-c/log_proto.h"
#include "protocols/rpc/common/packed-c/status.h"
#include "util.h"
#include <string.h>

static struct rpc_caller *rpc_caller;

void log_client_init(struct rpc_caller *caller)
{
	rpc_caller = caller;
}

log_status_t log_client_puts(const char *msg)
{
	uint8_t *request = NULL;
	uint8_t *response = NULL;
	size_t request_length = 0;
	size_t response_length = 0;
	size_t msg_length = 0;
	struct log_request *request_desc = NULL;
	rpc_call_handle handle = 0;
	rpc_status_t rpc_status = TS_RPC_CALL_ACCEPTED;
	log_status_t result = LOG_STATUS_SUCCESS;

	if (rpc_caller == NULL)
		return LOG_STATUS_GENERIC_ERROR;

	/* Validating input parameters */
	if (msg == NULL)
		return LOG_STATUS_INVALID_PARAMETER;

	msg_length = strlen(msg);

	/* Add one for null termination */
	if (ADD_OVERFLOW(msg_length, 1, &msg_length))
		return LOG_STATUS_INVALID_PARAMETER;

	if (ADD_OVERFLOW(sizeof(*request_desc), msg_length, &request_length))
		return LOG_STATUS_INVALID_PARAMETER;

	/* RPC call */
	handle = rpc_caller_begin(rpc_caller, &request, request_length);
	if (handle) {
		request_desc = (struct log_request *)request;
		memcpy(&request_desc->msg, msg, msg_length);
		request_desc->msg_length = msg_length;

		rpc_status = rpc_caller_invoke(rpc_caller, handle, TS_LOG_OPCODE_PUTS, &result,
					       &response, &response_length);

		if (rpc_status != TS_RPC_CALL_ACCEPTED)
			result = LOG_STATUS_GENERIC_ERROR;

		rpc_caller_end(rpc_caller, handle);
	} else
		result = LOG_STATUS_GENERIC_ERROR;

	return result;
}
