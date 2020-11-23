/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ffarpc_caller.h"
#include <components/rpc/ffarpc/endpoint/ffarpc_call_args.h>
#include <components/rpc/ffarpc/endpoint/ffarpc_call_ops.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <ffa_api.h>
#include <sp_memory_management.h>
#include <sp_rxtx.h>
#include <trace.h>
#include <stdbool.h>
#include <stdio.h>

uint8_t shared_buffer[4096] __aligned(4096);
extern uint16_t own_id; //TODO: replace this with nicer solution

static rpc_call_handle call_begin(void *context, uint8_t **req_buf, size_t req_len)
{
	struct ffarpc_caller *this_context = (struct ffarpc_caller *)context;
	rpc_call_handle handle = NULL;

	if (req_buf == NULL) {
		EMSG("call_begin(): invalid arguments");
		goto out;
	}

	if (this_context->is_call_transaction_in_progess) {
		EMSG("call_begin(): transaction already in progress");
		goto out;
	}

	this_context->is_call_transaction_in_progess = true;
	handle = this_context;

	if (req_len > 0) {
		this_context->req_buf = shared_buffer;
		*req_buf = this_context->req_buf;
		this_context->req_len = req_len;
	} else {
		*req_buf = NULL;
		this_context->req_buf = NULL;
		this_context->req_len = req_len;
	}
out:
	return handle;
}

static rpc_status_t call_invoke(void *context, rpc_call_handle handle, uint32_t opcode,
			    int *opstatus, uint8_t **resp_buf, size_t *resp_len)
{
	struct ffarpc_caller *this_context = (struct ffarpc_caller *)context;
	ffa_result res = FFA_OK;
	struct ffa_direct_msg req = { };
	struct ffa_direct_msg resp = { };
	rpc_status_t status = TS_RPC_ERROR_INTERNAL;

	if (handle != this_context || opstatus == NULL ||
	    resp_buf == NULL || resp_len == NULL) {
		EMSG("call_invoke(): invalid arguments");
		status = TS_RPC_ERROR_INVALID_PARAMETER;
		goto out;
	}

	if (!this_context->is_call_transaction_in_progess) {
		EMSG("call_invoke(): transaction was not started");
		status = TS_RPC_ERROR_NOT_READY;
		goto out;
	}

	req.destination_id = this_context->call_ep_id;
	req.source_id = own_id;
	req.args[FFA_CALL_ARGS_OPCODE] = opcode;
	//TODO: downcast problem?
	req.args[FFA_CALL_ARGS_REQ_DATA_LEN] = (uint32_t)this_context->req_len;

	res = ffa_msg_send_direct_req(req.source_id, req.destination_id,
				      req.args[0], req.args[1],
				      req.args[2], req.args[3],
				      req.args[4], &resp);

	if (res != FFA_OK) {
		EMSG("ffa_msg_send_direct_req(): error %"PRId32, res);
		goto out;
	}

	this_context->resp_len = (size_t)resp.args[FFA_CALL_ARGS_RESP_DATA_LEN];
	status = resp.args[FFA_CALL_ARGS_RESP_RPC_STATUS];
	*opstatus = resp.args[FFA_CALL_ARGS_RESP_OP_STATUS];

	if (this_context->resp_len > 0) {
		this_context->resp_buf = shared_buffer;
	} else {
		this_context->resp_buf = NULL;
	}

	*resp_buf = this_context->resp_buf;
	*resp_len = this_context->resp_len;
out:
	return status;
}

static void call_end(void *context, rpc_call_handle handle)
{
	struct ffarpc_caller *this_context = (struct ffarpc_caller *)context;

	if (handle != this_context) {
		EMSG("call_end(): invalid arguments");
		return;
	}

	this_context->req_buf = NULL;
	this_context->req_len = 0;
	this_context->resp_buf = NULL;
	this_context->resp_len = 0;
	this_context->is_call_transaction_in_progess = false;
}

struct rpc_caller *ffarpc_caller_init(struct ffarpc_caller *s)
{
	struct rpc_caller *base = &s->rpc_caller;

	base->context = s;
	base->call_begin = call_begin;
	base->call_invoke = call_invoke;
	base->call_end = call_end;

	s->call_ep_id = 0;
	s->shared_mem_handle = 0;
	s->shared_mem_required_size = sizeof(shared_buffer);
	s->req_buf = NULL;
	s->req_len = 0;
	s->resp_buf = NULL;
	s->resp_len = 0;
	s->is_call_transaction_in_progess = false;

	return base;
}

void ffarpc_caller_deinit(struct ffarpc_caller *s)
{
	s->rpc_caller.context = NULL;
	s->rpc_caller.call_begin = NULL;
	s->rpc_caller.call_invoke = NULL;
	s->rpc_caller.call_end = NULL;
}

uint32_t ffarpc_caller_discover(const uint8_t *uuid, uint16_t *sp_ids, uint32_t sp_max_cnt)
{
	ffa_result ffa_res;
	sp_result sp_res;
	const void *rx_buf_addr = NULL;
	size_t rx_buf_size = 0;
	uint32_t sp_cnt = 0;
	uint32_t i;

	if (uuid == NULL || sp_ids == NULL || sp_max_cnt == 0) {
		EMSG("ffarpc_caller_discover(): invalid arguments");
		goto out;
	}

	//TODO: not sure if this cast is acceptable
	ffa_res = ffa_partition_info_get((struct ffa_uuid *)uuid, &sp_cnt);
	if (ffa_res != FFA_OK) {
		EMSG("ffa_partition_info_get(): error %"PRId32, ffa_res);
		goto out;
	}

	sp_res = sp_rxtx_buffer_rx_get(&rx_buf_addr, &rx_buf_size);
	if (sp_res != SP_RESULT_OK) {
		EMSG("sp_rxtx_buffer_rx_get(): error %"PRId32, sp_res);
		goto out;
	}

	const struct ffa_partition_information *partitions =
		(const struct ffa_partition_information *)rx_buf_addr;

	for (i = 0; i < sp_cnt && i < sp_max_cnt; i++) {
		sp_ids[i] = partitions[i].partition_id;
	}

	ffa_res = ffa_rx_release();
	if (ffa_res != FFA_OK) {
		EMSG("ffa_rx_release(): error %"PRId32, ffa_res);
		goto out;
	}
out:
	return sp_cnt;
}

int ffarpc_caller_open(struct ffarpc_caller *s, uint16_t call_ep_id)
{
	//TODO: revise return type, error handling
	ffa_result ffa_res;
	struct ffa_direct_msg req = { };
	struct ffa_direct_msg resp = { };

	sp_result sp_res;
	struct sp_memory_descriptor desc = { };
	struct sp_memory_access_descriptor acc_desc = { };
	struct sp_memory_region region = { };

	uint64_t handle = 0;

	desc.sender_id = own_id;
	desc.memory_type = sp_memory_type_normal_memory;
	desc.mem_region_attr.normal_memory.cacheability = sp_cacheability_write_back;
	desc.mem_region_attr.normal_memory.shareability = sp_shareability_inner_shareable;

	acc_desc.data_access = sp_data_access_read_write;
	acc_desc.instruction_access = sp_instruction_access_not_executable;
	acc_desc.receiver_id = call_ep_id;

	region.address = shared_buffer;
	region.page_count = 1;

	sp_res = sp_memory_share(&desc, &acc_desc, 1, &region, 1, &handle);
	if (sp_res != SP_RESULT_OK) {
		EMSG("sp_memory_share(): error %"PRId32, sp_res);
		return -1;
	}

	req.source_id = own_id;
	req.destination_id = call_ep_id;
	req.args[FFA_CALL_ARGS_OPCODE] = FFA_CALL_OPCODE_SHARE_BUF;
	req.args[FFA_CALL_ARGS_SHARE_MEM_HANDLE_LSW] = (uint32_t)(handle & 0xffff);
	req.args[FFA_CALL_ARGS_SHARE_MEM_HANDLE_MSW] = (uint32_t)(handle >> 32);
	//TODO: downcast
	req.args[FFA_CALL_ARGS_SHARE_MEM_SIZE] = (uint32_t)(s->shared_mem_required_size);

	ffa_res = ffa_msg_send_direct_req(req.source_id, req.destination_id,
					  req.args[0], req.args[1],
					  req.args[2], req.args[3],
					  req.args[4], &resp);
	if (ffa_res != FFA_OK) {
		EMSG("ffa_msg_send_direct_req(): error %"PRId32, ffa_res);
		return -1;
	}

	s->call_ep_id = call_ep_id;
	s->shared_mem_handle = handle;

	return 0;
}

int ffarpc_caller_close(struct ffarpc_caller *s)
{
	//TODO: revise return type, error handling
	ffa_result ffa_res;
	struct ffa_direct_msg req = { 0 };
	struct ffa_direct_msg resp = { 0 };

	sp_result sp_res;
	uint32_t handle_lo, handle_hi;

	handle_lo = (uint32_t)(s->shared_mem_handle & UINT32_MAX);
	handle_hi = (uint32_t)(s->shared_mem_handle >> 32);

	req.source_id = own_id;
	req.destination_id = s->call_ep_id;
	req.args[FFA_CALL_ARGS_OPCODE] = FFA_CALL_OPCODE_UNSHARE_BUF;
	req.args[FFA_CALL_ARGS_SHARE_MEM_HANDLE_LSW] = handle_lo;
	req.args[FFA_CALL_ARGS_SHARE_MEM_HANDLE_MSW] = handle_hi;

	ffa_res = ffa_msg_send_direct_req(req.source_id, req.destination_id,
				      req.args[0], req.args[1],
				      req.args[2], req.args[3],
				      req.args[4], &resp);
	if (ffa_res != FFA_OK) {
		EMSG("ffa_msg_send_direct_req(): error %"PRId32, ffa_res);
		return -1;
	}

	sp_res = sp_memory_reclaim(s->shared_mem_handle, 0);
	if (sp_res != SP_RESULT_OK) {
		EMSG("sp_memory_reclaim(): error %"PRId32, sp_res);
		return -1;
	}

	s->call_ep_id = 0;
	s->shared_mem_handle = 0;

	return 0;
}
