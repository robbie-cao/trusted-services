/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "components/rpc/ffarpc/caller/sp/ffarpc_sp_call_args.h"
#include "ffarpc_call_ep.h"
#include "ffarpc_call_ops.h"
#include <ffa_api.h>
#include <sp_memory_management.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <trace.h>
#include <stddef.h>

/* TODO: remove this when own ID will be available in libsp */
extern uint16_t own_id;

static void set_resp_args(uint32_t *resp_args, uint32_t ifaceid_opcode, uint32_t data_len,
			  rpc_status_t rpc_status, rpc_opstatus_t opstatus)
{
	resp_args[SP_CALL_ARGS_IFACE_ID_OPCODE] = ifaceid_opcode;
	resp_args[SP_CALL_ARGS_RESP_DATA_LEN] = data_len;
	resp_args[SP_CALL_ARGS_RESP_RPC_STATUS] = rpc_status;
	resp_args[SP_CALL_ARGS_RESP_OP_STATUS] = (uint32_t)opstatus;
}

static void set_mgmt_resp_args(uint32_t *resp_args, uint32_t ifaceid_opcode,
			       rpc_status_t rpc_status)
{
	/*
	 * Sets arguments for responses that originate from the ffa_call_ep
	 * rather than from a higher layer service. These responses are not
	 * associated with a shared buffer for any additional message payload.
	 */
	set_resp_args(resp_args, ifaceid_opcode, 0, rpc_status, 0);
}

static int find_free_shm(struct ffa_call_ep *call_ep)
{
	int i;

	if (!call_ep)
		return -1;

	for (i = 0; i < NUM_MAX_SESS; i++)
		if (!call_ep->shmem_buf[i]) {
			DMSG("shm slot %u allocated for %p", i, call_ep);
			return i;
		}

	EMSG("shm slot allocation failed");
	return -1;
}

static int find_shm(struct ffa_call_ep *call_ep, uint16_t source_id)
{
	int i;

	if (!call_ep)
		return -1;

	for (i = 0; i < NUM_MAX_SESS; i++)
		if (call_ep->src_id[i] == source_id)
			return i;

	EMSG("shm not found for source 0x%x", source_id);
	return -1;
}

static void init_shmem_buf(struct ffa_call_ep *call_ep, uint16_t source_id,
			   const uint32_t *req_args, uint32_t *resp_args)
{
	sp_result sp_res = SP_RESULT_INTERNAL_ERROR;
	struct sp_memory_descriptor desc = { };
	struct sp_memory_access_descriptor acc_desc = { };
	struct sp_memory_region region = { };
	uint32_t in_region_count = 0;
	uint32_t out_region_count = 1;
	uint64_t handle = 0;
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	int idx = find_free_shm(call_ep);

	if (idx < 0) {
		EMSG("shm init error");
		goto out;
	}

	desc.sender_id = source_id;
	desc.memory_type = sp_memory_type_not_specified;
	desc.flags.transaction_type = sp_memory_transaction_type_share;
	acc_desc.receiver_id = own_id;
	acc_desc.data_access = sp_data_access_read_write;
	handle = req_args[SP_CALL_ARGS_SHARE_MEM_HANDLE_MSW];
	handle = (handle << 32) | req_args[SP_CALL_ARGS_SHARE_MEM_HANDLE_LSW];

	sp_res = sp_memory_retrieve(&desc, &acc_desc, &region, in_region_count,
				    &out_region_count, handle);

	if (sp_res == SP_RESULT_OK) {
		size_t shmem_size = (size_t)req_args[SP_CALL_ARGS_SHARE_MEM_SIZE];

		if (shmem_size > region.page_count * FFA_MEM_TRANSACTION_PAGE_SIZE) {
			/*
			 * The shared memory's size is smaller than the size
			 * value forwarded in the direct message argument.
			 */
			uint16_t endpoints[1] = { own_id };
			struct sp_memory_transaction_flags flags = {
				.zero_memory = false,
				.operation_time_slicing = false,
			};

			sp_res = sp_memory_relinquish(handle, endpoints, ARRAY_SIZE(endpoints),
						      &flags);
			if (sp_res)
				EMSG("memory relinquish error: %d", sp_res);

			rpc_status = TS_RPC_ERROR_INVALID_PARAMETER;
			goto out;
		}

		call_ep->shmem_buf[idx] = region.address;
		call_ep->shmem_buf_handle[idx] = handle;
		call_ep->shmem_buf_size[idx] = shmem_size;
		call_ep->src_id[idx] = source_id;
		rpc_status = TS_RPC_CALL_ACCEPTED;
	} else {
		EMSG("memory retrieve error: %d", sp_res);
	}

out:
	set_mgmt_resp_args(resp_args, req_args[SP_CALL_ARGS_IFACE_ID_OPCODE], rpc_status);
}

static void deinit_shmem_buf(struct ffa_call_ep *call_ep, uint16_t source_id,
			     const uint32_t *req_args, uint32_t *resp_args)
{
	sp_result sp_res = SP_RESULT_INTERNAL_ERROR;
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	uint64_t handle;
	uint16_t endpoints[1] = { own_id };
	uint32_t endpoint_cnt = 1;
	struct sp_memory_transaction_flags flags = {
		.zero_memory = false,
		.operation_time_slicing = false,
	};
	int idx = find_shm(call_ep, source_id);

	if (idx < 0) {
		EMSG("shm deinit error");
		goto out;
	}

	handle = call_ep->shmem_buf_handle[idx];

	sp_res = sp_memory_relinquish(handle, endpoints, endpoint_cnt, &flags);
	if (sp_res == SP_RESULT_OK) {
		call_ep->shmem_buf[idx] = NULL;
		call_ep->shmem_buf_handle[idx] = 0;
		call_ep->shmem_buf_size[idx] = 0;
		call_ep->src_id[idx] = 0xffff;
		rpc_status = TS_RPC_CALL_ACCEPTED;
	} else {
		EMSG("memory relinquish error: %d", sp_res);
	}

out:
	set_mgmt_resp_args(resp_args, req_args[SP_CALL_ARGS_IFACE_ID_OPCODE], rpc_status);
}

static void handle_service_msg(struct ffa_call_ep *call_ep, uint16_t source_id,
			       const uint32_t *req_args, uint32_t *resp_args)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INVALID_PARAMETER;
	struct call_req call_req;

	uint32_t ifaceid_opcode = req_args[SP_CALL_ARGS_IFACE_ID_OPCODE];
	int idx = find_shm(call_ep, source_id);

	call_req.caller_id = source_id;
	call_req.interface_id = FFA_CALL_ARGS_EXTRACT_IFACE(ifaceid_opcode);
	call_req.opcode = FFA_CALL_ARGS_EXTRACT_OPCODE(ifaceid_opcode);
	call_req.encoding = req_args[SP_CALL_ARGS_ENCODING];

	call_req.req_buf.data_len = req_args[SP_CALL_ARGS_REQ_DATA_LEN];
	call_req.resp_buf.data_len = 0;

	if (idx >= 0 && call_ep->shmem_buf[idx]) {
		/* A shared buffer is available for call parameters */
		if (call_req.req_buf.data_len > call_ep->shmem_buf_size[idx])
			goto out;

		call_req.req_buf.data = call_ep->shmem_buf[idx];
		call_req.req_buf.size = call_ep->shmem_buf_size[idx];

		call_req.resp_buf.data = call_ep->shmem_buf[idx];
		call_req.resp_buf.size = call_ep->shmem_buf_size[idx];
	}
	else if (call_req.req_buf.data_len == 0) {
		/* No shared buffer so only allow calls with no request data */
		call_req.req_buf.data = NULL;
		call_req.req_buf.size = 0;

		call_req.resp_buf.data = NULL;
		call_req.resp_buf.size = 0;
	}
	else {
		/*
		 * Caller has specified non-zero length request data but there is
		 * no shared buffer to carry the request data.
		 */
		goto out;
	}

	rpc_status = rpc_interface_receive(call_ep->iface, &call_req);

out:
	set_resp_args(resp_args,
		      ifaceid_opcode,
		      call_req.resp_buf.data_len,
		      rpc_status,
		      call_req.opstatus);
}

static void handle_mgmt_msg(struct ffa_call_ep *call_ep, uint16_t source_id,
			    const uint32_t *req_args, uint32_t *resp_args)
{
	uint32_t ifaceid_opcode = req_args[SP_CALL_ARGS_IFACE_ID_OPCODE];
	uint32_t opcode = FFA_CALL_ARGS_EXTRACT_OPCODE(ifaceid_opcode);

	switch (opcode) {
	case FFA_CALL_OPCODE_SHARE_BUF:
		init_shmem_buf(call_ep, source_id, req_args, resp_args);
		break;
	case FFA_CALL_OPCODE_UNSHARE_BUF:
		deinit_shmem_buf(call_ep, source_id, req_args, resp_args);
		break;
	default:
		set_mgmt_resp_args(resp_args, ifaceid_opcode, TS_RPC_ERROR_INVALID_OPCODE);
		break;
	}
}

void ffa_call_ep_init(struct ffa_call_ep *ffa_call_ep, struct rpc_interface *iface)
{
	int i;

	ffa_call_ep->iface = iface;

	for (i = 0; i < NUM_MAX_SESS; i++) {
		ffa_call_ep->shmem_buf_handle[i] = 0;
		ffa_call_ep->shmem_buf_size[i] = 0;
		ffa_call_ep->shmem_buf[i] = NULL;
		ffa_call_ep->src_id[i] = 0xffff;
	}
}

void ffa_call_ep_receive(struct ffa_call_ep *call_ep,
			 const struct sp_msg *req_msg,
			 struct sp_msg *resp_msg)
{
	const uint32_t *req_args = req_msg->args.args32;
	uint32_t *resp_args = resp_msg->args.args32;

	uint16_t source_id = req_msg->source_id;
	uint32_t ifaceid_opcode = req_args[SP_CALL_ARGS_IFACE_ID_OPCODE];

	if (FFA_CALL_ARGS_EXTRACT_IFACE(ifaceid_opcode) == FFA_CALL_MGMT_IFACE_ID) {
		/* It's an RPC layer management request */
		handle_mgmt_msg(call_ep, source_id, req_args, resp_args);
	} else {
		/* Assume anything else is a service request */
		handle_service_msg(call_ep, source_id, req_args, resp_args);
	}
}
