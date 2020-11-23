/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <rpc_caller.h>
#include <stdint.h>

rpc_call_handle rpc_caller_begin(struct rpc_caller *s,
								uint8_t **req_buf, size_t req_len)
{
	return s->call_begin(s->context, req_buf, req_len);
}

rpc_status_t rpc_caller_invoke(struct rpc_caller *s, rpc_call_handle handle,
			uint32_t opcode, int *opstatus, uint8_t **resp_buf, size_t *resp_len)
{
	return s->call_invoke(s->context, handle, opcode, opstatus, resp_buf, resp_len);
}

void rpc_caller_end(struct rpc_caller *s, rpc_call_handle handle)
{
	s->call_end(s->context, handle);
}
