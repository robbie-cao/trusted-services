// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (C) 2020-2022, Arm Limited
 */

#include "sel1_sp_ffarpc_call_ep.h"
#include "components/rpc/ffarpc/common/ffarpc_sp_call_args.h"
#include "components/rpc/ffarpc/common/ffarpc_call_ops.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <kernel/pseudo_sp.h>
#include <trace.h>
#include <stddef.h>

static void set_resp_args(uint32_t *resp_args, uint32_t ifaceid_opcode,
			  uint32_t data_len, rpc_status_t rpc_status,
			  uint32_t opstatus)
{
	resp_args[SP_CALL_ARGS_IFACE_ID_OPCODE] = ifaceid_opcode;
	resp_args[SP_CALL_ARGS_RESP_DATA_LEN] = data_len;
	resp_args[SP_CALL_ARGS_RESP_RPC_STATUS] = rpc_status;
	resp_args[SP_CALL_ARGS_RESP_OP_STATUS] = opstatus;
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
	int i = 0;

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
	int i = 0;

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
	uint64_t handle = 0;
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	int idx = find_free_shm(call_ep);
	void *mem = NULL;

	if (idx < 0) {
		EMSG("shm init error");
		goto out;
	}

	handle = req_args[SP_CALL_ARGS_SHARE_MEM_HANDLE_MSW];
	handle = (handle << 32) | req_args[SP_CALL_ARGS_SHARE_MEM_HANDLE_LSW];

	mem = pseudo_sp_retrieve_mem(handle);
	if (mem) {
		call_ep->shmem_buf[idx] = mem;
		call_ep->shmem_buf_handle[idx] = handle;
		call_ep->shmem_buf_size[idx] =
			(size_t)req_args[SP_CALL_ARGS_SHARE_MEM_SIZE];
		call_ep->src_id[idx] = source_id;
		rpc_status = TS_RPC_CALL_ACCEPTED;
	} else {
		EMSG("memory retrieve error");
	}

out:
	set_mgmt_resp_args(resp_args, req_args[SP_CALL_ARGS_IFACE_ID_OPCODE],
			   rpc_status);
}

static void deinit_shmem_buf(struct ffa_call_ep *call_ep, uint16_t source_id,
			     const uint32_t *req_args, uint32_t *resp_args)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	int idx = find_shm(call_ep, source_id);

	if (idx < 0) {
		EMSG("shm deinit error");
		goto out;
	}

	call_ep->shmem_buf[idx] = NULL;
	call_ep->shmem_buf_handle[idx] = 0;
	call_ep->shmem_buf_size[idx] = 0;
	call_ep->src_id[idx] = 0xffff;
	rpc_status = TS_RPC_CALL_ACCEPTED;

out:
	set_mgmt_resp_args(resp_args, req_args[SP_CALL_ARGS_IFACE_ID_OPCODE],
			   rpc_status);
}

static void handle_service_msg(struct ffa_call_ep *call_ep, uint16_t source_id,
			       const uint32_t *req_args, uint32_t *resp_args)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	struct call_req call_req = { 0 };
	uint32_t ifaceid_opcode = req_args[SP_CALL_ARGS_IFACE_ID_OPCODE];
	int idx = find_shm(call_ep, source_id);

	if (idx < 0) {
		EMSG("handle service msg error");
		goto out;
	}

	call_req.caller_id = source_id;
	call_req.interface_id = FFA_CALL_ARGS_EXTRACT_IFACE(ifaceid_opcode);
	call_req.opcode = FFA_CALL_ARGS_EXTRACT_OPCODE(ifaceid_opcode);
	call_req.encoding = req_args[SP_CALL_ARGS_ENCODING];

	call_req.req_buf.data = call_ep->shmem_buf[idx];
	call_req.req_buf.data_len = req_args[SP_CALL_ARGS_REQ_DATA_LEN];
	call_req.req_buf.size = call_ep->shmem_buf_size[idx];

	call_req.resp_buf.data = call_ep->shmem_buf[idx];
	call_req.resp_buf.data_len = 0;
	call_req.resp_buf.size = call_ep->shmem_buf_size[idx];

	rpc_status = rpc_interface_receive(call_ep->iface, &call_req);

out:
	set_resp_args(resp_args, ifaceid_opcode, call_req.resp_buf.data_len,
		      rpc_status, call_req.opstatus);
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
		set_mgmt_resp_args(resp_args, ifaceid_opcode,
				   TS_RPC_ERROR_INVALID_OPCODE);
		break;
	}
}

void ffa_call_ep_init(struct ffa_call_ep *ffa_call_ep,
		      struct rpc_interface *iface)
{
	int i = 0;

	ffa_call_ep->iface = iface;

	for (i = 0; i < NUM_MAX_SESS; i++) {
		ffa_call_ep->shmem_buf_handle[i] = 0;
		ffa_call_ep->shmem_buf_size[i] = 0;
		ffa_call_ep->shmem_buf[i] = NULL;
		ffa_call_ep->src_id[i] = 0xffff;
	}
}

void ffa_call_ep_receive(struct ffa_call_ep *call_ep,
			 const struct thread_smc_args *req_msg,
			 struct thread_smc_args *resp_msg)
{
	const uint32_t req_args[4] = { req_msg->a4, req_msg->a5, req_msg->a6,
				       req_msg->a7 };
	uint32_t resp_args[4] = { 0 };
	int idx = 0;
	uint16_t source_id = (req_msg->a1 >> 16) & 0xffff;
	uint32_t ifaceid_opcode = req_args[SP_CALL_ARGS_IFACE_ID_OPCODE];

	if (FFA_CALL_ARGS_EXTRACT_IFACE(ifaceid_opcode) ==
	    FFA_CALL_MGMT_IFACE_ID) {
		/* It's an RPC layer management request */
		handle_mgmt_msg(call_ep, source_id, req_args, resp_args);
	} else {
		/*
		 * Assume anything else is a service request. Service requests
		 * rely on a buffer being shared from the requesting client.
		 * If it hasn't been set-up, fail the request.
		 */
		idx = find_shm(call_ep, source_id);

		if (idx >= 0 && call_ep->shmem_buf[idx]) {
			handle_service_msg(call_ep, source_id, req_args,
					   resp_args);
		} else {
			EMSG("shared buffer not found or NULL");
			set_mgmt_resp_args(resp_args, ifaceid_opcode,
					   TS_RPC_ERROR_NOT_READY);
		}
	}

	resp_msg->a4 = resp_args[0];
	resp_msg->a5 = resp_args[1];
	resp_msg->a6 = resp_args[2];
	resp_msg->a7 = resp_args[3];
}

void *ffa_call_ep_get_buffer(struct ffa_call_ep *call_ep, uint16_t source_id,
			     size_t *buffer_size)
{
	int idx = find_shm(call_ep, source_id);

	if (idx < 0 || !call_ep->shmem_buf[idx])
		return NULL;

	*buffer_size = call_ep->shmem_buf_size[idx];
	return call_ep->shmem_buf[idx];
}
