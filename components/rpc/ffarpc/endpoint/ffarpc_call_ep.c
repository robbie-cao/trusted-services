/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ffarpc_call_args.h"
#include "ffarpc_call_ep.h"
#include "ffarpc_call_ops.h"
#include <ffa_api.h>
#include <sp_memory_management.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <trace.h>
#include <stddef.h>

/* TODO: remove this when own ID will be available in libsp */
extern uint16_t own_id;

static void set_resp_args(uint32_t *resp_args, uint32_t opcode, uint32_t data_len,
			  rpc_status_t rpc_status, uint32_t opstatus)
{
	resp_args[FFA_CALL_ARGS_OPCODE] = opcode;
	resp_args[FFA_CALL_ARGS_RESP_DATA_LEN] = data_len;
	resp_args[FFA_CALL_ARGS_RESP_RPC_STATUS] = rpc_status;
	resp_args[FFA_CALL_ARGS_RESP_OP_STATUS] = opstatus;
}

static void set_mgmt_resp_args(uint32_t *resp_args, uint32_t opcode,
			       rpc_status_t rpc_status)
{
	/*
	 * Sets arguments for responses that originate from the ffa_call_ep
	 * rather than from a higher layer service. These responses are not
	 * associated with a shared buffer for any additional message payload.
	 */
	set_resp_args(resp_args, opcode, 0, rpc_status, 0);
}

static void init_shmem_buf(struct ffa_call_ep *call_ep, uint16_t source_id,
			   const uint32_t *req_args, uint32_t *resp_args)
{
	sp_result sp_res = SP_RESULT_INTERNAL_ERROR;
	struct sp_memory_descriptor desc = { };
	struct sp_memory_access_descriptor acc_desc = { };
	struct sp_memory_region region = { };
	uint32_t in_region_count = 1;
	uint32_t out_region_count = 1;
	uint64_t handle = 0;
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;

	desc.sender_id = source_id;
	desc.memory_type = sp_memory_type_not_specified;
	desc.flags.transaction_type = sp_memory_transaction_type_share;
	acc_desc.receiver_id = own_id;
	acc_desc.data_access = sp_data_access_read_write;
	handle = req_args[FFA_CALL_ARGS_SHARE_MEM_HANDLE_MSW];
	handle = (handle << 32) | req_args[FFA_CALL_ARGS_SHARE_MEM_HANDLE_LSW];

	sp_res = sp_memory_retrieve(&desc, &acc_desc, &region, in_region_count,
				    &out_region_count, handle);

	if (sp_res == SP_RESULT_OK) {
		call_ep->shmem_buf = region.address;
		call_ep->shmem_buf_handle = handle;
		call_ep->shmem_buf_size = (size_t)req_args[FFA_CALL_ARGS_SHARE_MEM_SIZE];
		rpc_status = TS_RPC_CALL_ACCEPTED;
	} else {
		EMSG("memory retrieve error: %d", sp_res);
	}

	set_mgmt_resp_args(resp_args, req_args[FFA_CALL_ARGS_OPCODE], rpc_status);
}

static void deinit_shmem_buf(struct ffa_call_ep *call_ep, const uint32_t *req_args,
			     uint32_t *resp_args)
{
	sp_result sp_res = SP_RESULT_INTERNAL_ERROR;
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	uint64_t handle = call_ep->shmem_buf_handle;
	uint16_t endpoints[1] = { own_id };
	uint32_t endpoint_cnt = 1;
	struct sp_memory_transaction_flags flags = {
		.zero_memory = false,
		.operation_time_slicing = false,
	};

	sp_res = sp_memory_relinquish(handle, endpoints, endpoint_cnt, &flags);
	if (sp_res == SP_RESULT_OK) {
		call_ep->shmem_buf = NULL;
		call_ep->shmem_buf_handle = 0;
		call_ep->shmem_buf_size = 0;
		rpc_status = TS_RPC_CALL_ACCEPTED;
	} else {
		EMSG("memory relinquish error: %d", sp_res);
	}

	set_mgmt_resp_args(resp_args, req_args[FFA_CALL_ARGS_OPCODE], rpc_status);
}

static void handle_service_msg(struct ffa_call_ep *call_ep, uint16_t source_id,
			       const uint32_t *req_args, uint32_t *resp_args)
{
	rpc_status_t rpc_status;
	struct call_req call_req;

	call_req.caller_id = source_id;
	call_req.opcode = req_args[FFA_CALL_ARGS_OPCODE];

	call_req.req_buf.data = call_ep->shmem_buf;
	call_req.req_buf.data_len = req_args[FFA_CALL_ARGS_REQ_DATA_LEN];
	call_req.req_buf.size = call_ep->shmem_buf_size;

	call_req.resp_buf.data = call_ep->shmem_buf;
	call_req.resp_buf.data_len = 0;
	call_req.resp_buf.size = call_ep->shmem_buf_size;

	rpc_status = call_ep_receive(call_ep->call_ep, &call_req);

	set_resp_args(resp_args,
		      call_req.opcode,
		      call_req.resp_buf.data_len,
		      rpc_status,
		      call_req.opstatus);
}

static void handle_mgmt_msg(struct ffa_call_ep *call_ep, uint16_t source_id,
			    const uint32_t *req_args, uint32_t *resp_args)
{
	uint32_t opcode = req_args[FFA_CALL_ARGS_OPCODE];

	/*
	 * TODO: shouldn't this be used to keep track of multiple
	 * shared buffers for different endpoints?
	 */
	(void)source_id;

	switch (opcode) {
	case FFA_CALL_OPCODE_SHARE_BUF:
		init_shmem_buf(call_ep, source_id, req_args, resp_args);
		break;
	case FFA_CALL_OPCODE_UNSHARE_BUF:
		deinit_shmem_buf(call_ep, req_args, resp_args);
		break;
	default:
		set_mgmt_resp_args(resp_args, opcode, TS_RPC_ERROR_INVALID_OPCODE);
		break;
	}
}

void ffa_call_ep_init(struct ffa_call_ep *ffa_call_ep, struct call_ep *call_ep)
{
	ffa_call_ep->call_ep = call_ep;
	ffa_call_ep->shmem_buf_handle = 0;
	ffa_call_ep->shmem_buf_size = 0;
	ffa_call_ep->shmem_buf = NULL;
}

void ffa_call_ep_receive(struct ffa_call_ep *call_ep,
			 const struct ffa_direct_msg *req_msg,
			 struct ffa_direct_msg *resp_msg)
{
	const uint32_t *req_args = req_msg->args;
	uint32_t *resp_args = resp_msg->args;

	uint16_t source_id = req_msg->source_id;
	uint32_t opcode = req_args[FFA_CALL_ARGS_OPCODE];

	if (FFA_CALL_OPCODE_IS_MGMT(opcode)) {
		/* It's an RPC layer management request */
		handle_mgmt_msg(call_ep, source_id, req_args, resp_args);
	} else {
		/*
		 * Assume anything else is a service request. Service requests
		 * rely on a buffer being shared from the requesting client.
		 * If it hasn't been set-up, fail the request.
		 */
		if (call_ep->shmem_buf)
			handle_service_msg(call_ep, source_id, req_args, resp_args);
		else
			set_mgmt_resp_args(resp_args, opcode, TS_RPC_ERROR_NOT_READY);
	}
}