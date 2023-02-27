/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2023, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "psa_ipc_caller.h"
#include <openamp_messenger_api.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <rpc_caller.h>
#include <rpc_status.h>
#include <trace.h>
#include <errno.h>
#include <stdint.h>

static rpc_call_handle psa_ipc_call_begin(void *context, uint8_t **req_buf,
					  size_t req_len)
{
	struct psa_ipc_caller *psa_ipc = context;
	struct openamp_messenger *openamp = &psa_ipc->openamp;
	rpc_call_handle handle;
	int ret;

	ret = openamp_messenger_call_begin(openamp, req_buf, req_len);
	if (ret < 0)
		return NULL;

	handle = psa_ipc;

	return handle;
}

static rpc_status_t psa_ipc_call_invoke(void *context, rpc_call_handle handle,
					uint32_t opcode, long int *opstatus,
					uint8_t **resp_buf, size_t *resp_len)
{
	struct psa_ipc_caller *psa_ipc = context;
	struct openamp_messenger *openamp = &psa_ipc->openamp;
	int ret;

	(void)opcode;

	ret = openamp_messenger_call_invoke(openamp, resp_buf, resp_len);
	if (ret == -EINVAL)
		return TS_RPC_ERROR_INVALID_PARAMETER;
	if (ret == -ENOTCONN)
		return TS_RPC_ERROR_NOT_READY;
	if (ret < 0)
		return TS_RPC_ERROR_INTERNAL;

	*opstatus = 0;

	return TS_RPC_CALL_ACCEPTED;
}

static void psa_ipc_call_end(void *context, rpc_call_handle handle)
{
	struct psa_ipc_caller *psa_ipc = context;

	if (!psa_ipc || psa_ipc != handle) {
		EMSG("psa_ipc: call_end: invalid arguments");
		return;
	}

	openamp_messenger_call_end(&psa_ipc->openamp);
}


void *psa_ipc_phys_to_virt(void *context, void *pa)
{
	struct psa_ipc_caller *psa_ipc = context;
	struct openamp_messenger *openamp = &psa_ipc->openamp;

	return openamp_messenger_phys_to_virt(openamp, pa);
}

void *psa_ipc_virt_to_phys(void *context, void *va)
{
	struct psa_ipc_caller *psa_ipc = context;
	struct openamp_messenger *openamp = &psa_ipc->openamp;

	return openamp_messenger_virt_to_phys(openamp, va);
}

struct rpc_caller *psa_ipc_caller_init(struct psa_ipc_caller *psa_ipc)
{
	struct rpc_caller *rpc = &psa_ipc->rpc_caller;
	int ret;

	ret = openamp_messenger_init(&psa_ipc->openamp);
	if (ret < 0)
		return NULL;

	rpc_caller_init(rpc, &psa_ipc->rpc_caller);
	rpc->call_begin = psa_ipc_call_begin;
	rpc->call_invoke = psa_ipc_call_invoke;
	rpc->call_end = psa_ipc_call_end;

	return rpc;
}

struct rpc_caller *psa_ipc_caller_deinit(struct psa_ipc_caller *psa_ipc)
{
	struct rpc_caller *rpc = &psa_ipc->rpc_caller;

	rpc->context = NULL;
	rpc->call_begin = NULL;
	rpc->call_invoke = NULL;
	rpc->call_end = NULL;

	return rpc;
}
