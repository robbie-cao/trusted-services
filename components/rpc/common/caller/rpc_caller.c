/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <rpc_caller.h>
#include <stdint.h>
#include <protocols/rpc/common/packed-c/encoding.h>

void rpc_caller_init(struct rpc_caller *s, void *context)
{
	s->context = context;

	/* The default encoding scheme - may be overridden by a client */
	s->encoding = TS_RPC_ENCODING_PACKED_C;
}

void rpc_caller_set_encoding_scheme(struct rpc_caller *s, uint32_t encoding)
{
	s->encoding = encoding;
}

rpc_call_handle rpc_caller_begin(struct rpc_caller *s,
								uint8_t **req_buf, size_t req_len)
{
	return s->call_begin(s->context, req_buf, req_len);
}

rpc_status_t rpc_caller_invoke(struct rpc_caller *s, rpc_call_handle handle,
			uint32_t opcode, rpc_opstatus_t *opstatus, uint8_t **resp_buf, size_t *resp_len)
{
	return s->call_invoke(s->context, handle, opcode, opstatus, resp_buf, resp_len);
}

void rpc_caller_end(struct rpc_caller *s, rpc_call_handle handle)
{
	s->call_end(s->context, handle);
}
