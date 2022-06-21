/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ffarpc_caller.h"
#include "ffarpc_sp_call_args.h"
#include <components/rpc/ffarpc/endpoint/ffarpc_call_ops.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <ffa_api.h>
#include <sp_memory_management.h>
#include <sp_messaging.h>
#include <sp_rxtx.h>
#include <trace.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

uint8_t shared_buffer[4096] __aligned(4096);

static rpc_call_handle call_begin(void *context, uint8_t **req_buf, size_t req_len)
{
	struct ffarpc_caller *this_context = (struct ffarpc_caller *)context;
	rpc_call_handle handle = NULL;

	if (context == NULL || req_buf == NULL) {
		EMSG("invalid arguments");
		goto out;
	}

	if (this_context->is_call_transaction_in_progess) {
		EMSG("transaction already in progress");
		goto out;
	}

	if (req_len > this_context->shared_mem_required_size) {
		EMSG("req_len too big");
		goto out;
	}

	if (this_context->dest_partition_id == 0) {
		EMSG("the caller is not open");
		goto out;
	}

	this_context->is_call_transaction_in_progess = true;
	handle = this_context;

	*req_buf = (req_len > 0) ? shared_buffer : NULL;

	this_context->req_buf = *req_buf;
	this_context->req_len = req_len;

out:
	return handle;
}

static rpc_status_t call_invoke(void *context, rpc_call_handle handle, uint32_t opcode,
				rpc_opstatus_t *opstatus, uint8_t **resp_buf, size_t *resp_len)
{
	struct ffarpc_caller *this_context = (struct ffarpc_caller *)context;
	sp_result sp_res = SP_RESULT_OK;
	struct sp_msg req = { 0 };
	struct sp_msg resp = { 0 };
	rpc_status_t status = TS_RPC_ERROR_INTERNAL;

	if (context == NULL || handle != this_context || opstatus == NULL ||
	    resp_buf == NULL || resp_len == NULL) {
		EMSG("invalid arguments");
		status = TS_RPC_ERROR_INVALID_PARAMETER;
		goto out;
	}

	if (!this_context->is_call_transaction_in_progess) {
		EMSG("transaction was not started");
		status = TS_RPC_ERROR_NOT_READY;
		goto out;
	}

	if (this_context->req_len > UINT32_MAX) {
		EMSG("req_len is too large");
		goto out;
	}

	req.destination_id = this_context->dest_partition_id;
	req.source_id = this_context->own_id;
	req.is_64bit_message = false;
	req.args.args32[SP_CALL_ARGS_IFACE_ID_OPCODE] =
		FFA_CALL_ARGS_COMBINE_IFACE_ID_OPCODE(this_context->dest_iface_id, opcode);
	req.args.args32[SP_CALL_ARGS_REQ_DATA_LEN] = (uint32_t)this_context->req_len;
	req.args.args32[SP_CALL_ARGS_ENCODING] = this_context->rpc_caller.encoding;

	/* Initialise the caller ID.  Depending on the call path, this may
	 * be overridden by a higher privilege execution level, based on its
	 * perspective of the caller identity.
	 */
	req.args.args32[SP_CALL_ARGS_CALLER_ID] = 0;

	sp_res = sp_msg_send_direct_req(&req, &resp);
	if (sp_res != SP_RESULT_OK) {
		EMSG("sp_msg_send_direct_req(): error %"PRId32, sp_res);
		goto out;
	}

	this_context->resp_len = (size_t)resp.args.args32[SP_CALL_ARGS_RESP_DATA_LEN];
	if (this_context->resp_len > this_context->shared_mem_required_size) {
		EMSG("invalid response length");
		goto out;
	}

	status = resp.args.args32[SP_CALL_ARGS_RESP_RPC_STATUS];
	*opstatus = (rpc_status_t)((int32_t)resp.args.args32[SP_CALL_ARGS_RESP_OP_STATUS]);

	if (this_context->resp_len > 0)
		this_context->resp_buf = shared_buffer;
	else
		this_context->resp_buf = NULL;

	*resp_buf = this_context->resp_buf;
	*resp_len = this_context->resp_len;
out:
	return status;
}

static void call_end(void *context, rpc_call_handle handle)
{
	struct ffarpc_caller *this_context = (struct ffarpc_caller *)context;

	if (context == NULL || handle != this_context) {
		EMSG("invalid arguments");
		return;
	}

	this_context->req_buf = NULL;
	this_context->req_len = 0;
	this_context->resp_buf = NULL;
	this_context->resp_len = 0;
	this_context->is_call_transaction_in_progess = false;
}

struct rpc_caller *ffarpc_caller_init(struct ffarpc_caller *caller, uint16_t own_id)
{
	struct rpc_caller *base = &caller->rpc_caller;

	rpc_caller_init(base, caller);
	base->call_begin = call_begin;
	base->call_invoke = call_invoke;
	base->call_end = call_end;

	caller->own_id = own_id;
	caller->dest_partition_id = 0;
	caller->dest_iface_id = 0;
	caller->shared_mem_handle = 0;
	caller->shared_mem_required_size = sizeof(shared_buffer);
	caller->req_buf = NULL;
	caller->req_len = 0;
	caller->resp_buf = NULL;
	caller->resp_len = 0;
	caller->is_call_transaction_in_progess = false;

	return base;
}

void ffarpc_caller_deinit(struct ffarpc_caller *caller)
{
	caller->rpc_caller.context = NULL;
	caller->rpc_caller.call_begin = NULL;
	caller->rpc_caller.call_invoke = NULL;
	caller->rpc_caller.call_end = NULL;
}

uint32_t ffarpc_caller_discover(const uint8_t *uuid, uint16_t *sp_ids, uint32_t sp_max_cnt)
{
	ffa_result ffa_res = FFA_OK;
	sp_result sp_res = SP_RESULT_OK;
	const void *rx_buf_addr = NULL;
	size_t rx_buf_size = 0;
	const struct ffa_partition_information *partitions = NULL;
	uint32_t sp_cnt = 0;
	uint32_t i = 0;

	if (uuid == NULL || sp_ids == NULL || sp_max_cnt == 0) {
		EMSG("invalid arguments");
		return 0;
	}

	ffa_res = ffa_partition_info_get((struct ffa_uuid *)uuid, &sp_cnt);
	if (ffa_res != FFA_OK) {
		EMSG("ffa_partition_info_get(): error %"PRId32, ffa_res);
		return 0;
	}

	sp_res = sp_rxtx_buffer_rx_get(&rx_buf_addr, &rx_buf_size);
	if (sp_res != SP_RESULT_OK) {
		EMSG("sp_rxtx_buffer_rx_get(): error %"PRId32, sp_res);
		return 0;
	}

	partitions = (const struct ffa_partition_information *)rx_buf_addr;
	for (i = 0; i < sp_cnt && i < sp_max_cnt; i++)
		sp_ids[i] = partitions[i].partition_id;

	ffa_res = ffa_rx_release();
	if (ffa_res != FFA_OK) {
		EMSG("ffa_rx_release(): error %"PRId32, ffa_res);
		return 0;
	}

	return sp_cnt;
}

int ffarpc_caller_open(struct ffarpc_caller *caller, uint16_t dest_partition_id,
		       uint16_t dest_iface_id)
{
	sp_result sp_res = SP_RESULT_OK;
	struct sp_msg req = { 0 };
	struct sp_msg resp = { 0 };

	struct sp_memory_descriptor desc = { 0 };
	struct sp_memory_access_descriptor acc_desc = { 0 };
	struct sp_memory_region region = { 0 };

	uint64_t handle = 0;

	rpc_status_t status = TS_RPC_ERROR_INTERNAL;

	if (caller->dest_partition_id) {
		EMSG("the caller is already open");
		return -1;
	}

	if (caller->shared_mem_required_size > UINT32_MAX) {
		EMSG("memory is too large to share");
		return -1;
	}

	desc.sender_id = caller->own_id;
	desc.memory_type = sp_memory_type_normal_memory;
	desc.mem_region_attr.normal_memory.cacheability = sp_cacheability_write_back;
	desc.mem_region_attr.normal_memory.shareability = sp_shareability_inner_shareable;

	acc_desc.data_access = sp_data_access_read_write;
	acc_desc.instruction_access = sp_instruction_access_not_specified;
	acc_desc.receiver_id = dest_partition_id;

	region.address = shared_buffer;
	region.page_count = 1;

	sp_res = sp_memory_share(&desc, &acc_desc, 1, &region, 1, &handle);
	if (sp_res != SP_RESULT_OK) {
		EMSG("sp_memory_share(): error %"PRId32, sp_res);
		return -1;
	}

	req.source_id = caller->own_id;
	req.destination_id = dest_partition_id;
	req.is_64bit_message = false;
	req.args.args32[SP_CALL_ARGS_IFACE_ID_OPCODE] =
		FFA_CALL_ARGS_COMBINE_IFACE_ID_OPCODE(FFA_CALL_MGMT_IFACE_ID, FFA_CALL_OPCODE_SHARE_BUF);
	req.args.args32[SP_CALL_ARGS_SHARE_MEM_HANDLE_LSW] = (uint32_t)(handle & UINT32_MAX);
	req.args.args32[SP_CALL_ARGS_SHARE_MEM_HANDLE_MSW] = (uint32_t)(handle >> 32);
	req.args.args32[SP_CALL_ARGS_SHARE_MEM_SIZE] = (uint32_t)(caller->shared_mem_required_size);

	sp_res = sp_msg_send_direct_req(&req, &resp);
	if (sp_res != SP_RESULT_OK) {
		EMSG("sp_msg_send_direct_req(): error %"PRId32, sp_res);
		return -1;
	}

	status = (rpc_status_t)resp.args.args32[SP_CALL_ARGS_RESP_RPC_STATUS];
	if (status != TS_RPC_CALL_ACCEPTED) {
		EMSG("RPC endpoint error: %"PRId32, status);

		sp_res = sp_memory_reclaim(handle, 0);
		if (sp_res != SP_RESULT_OK)
			EMSG("sp_memory_reclaim(): error %"PRId32, sp_res);

		return -1;
	}

	caller->dest_partition_id = dest_partition_id;
	caller->dest_iface_id = dest_iface_id;
	caller->shared_mem_handle = handle;

	return 0;
}

int ffarpc_caller_close(struct ffarpc_caller *caller)
{
	sp_result sp_res = SP_RESULT_OK;
	struct sp_msg req = { 0 };
	struct sp_msg resp = { 0 };
	uint32_t handle_lo = 0, handle_hi = 0;
	rpc_status_t status = TS_RPC_ERROR_INTERNAL;

	if (caller->dest_partition_id == 0) {
		EMSG("the caller is already closed");
		return -1;
	}

	handle_lo = (uint32_t)(caller->shared_mem_handle & UINT32_MAX);
	handle_hi = (uint32_t)(caller->shared_mem_handle >> 32);

	req.source_id = caller->own_id;
	req.destination_id = caller->dest_partition_id;
	req.is_64bit_message = false;
	req.args.args32[SP_CALL_ARGS_IFACE_ID_OPCODE] =
		FFA_CALL_ARGS_COMBINE_IFACE_ID_OPCODE(FFA_CALL_MGMT_IFACE_ID, FFA_CALL_OPCODE_UNSHARE_BUF);
	req.args.args32[SP_CALL_ARGS_SHARE_MEM_HANDLE_LSW] = handle_lo;
	req.args.args32[SP_CALL_ARGS_SHARE_MEM_HANDLE_MSW] = handle_hi;

	sp_res = sp_msg_send_direct_req(&req, &resp);
	if (sp_res != SP_RESULT_OK) {
		EMSG("sp_msg_send_direct_req(): error %"PRId32, sp_res);
		return -1;
	}

	status = (rpc_status_t)resp.args.args32[SP_CALL_ARGS_RESP_RPC_STATUS];
	if (status) {
		/*
		 * The RPC endpoint failed to relinquish the shared memory but
		 * still worth trying to reclaim the memory beside emiting an
		 * error message.
		 */
		EMSG("RPC endpoint error %"PRId32, status);
	}

	sp_res = sp_memory_reclaim(caller->shared_mem_handle, 0);
	if (sp_res != SP_RESULT_OK) {
		EMSG("sp_memory_reclaim(): error %"PRId32, sp_res);
		return -1;
	}

	caller->dest_partition_id = 0;
	caller->dest_iface_id = 0;
	caller->shared_mem_handle = 0;

	return 0;
}
