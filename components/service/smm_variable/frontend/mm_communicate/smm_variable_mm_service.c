// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include "protocols/common/mm/mm_smc.h"
#include "protocols/service/smm_variable/smm_variable_proto.h"
#include "smm_variable_mm_service.h"
#include <assert.h>

struct smm_variable_rpc_context {
	struct rpc_interface *smm_variable_rpc_interface;
};

static int32_t convert_rpc_status(rpc_status_t rpc_status)
{
	switch (rpc_status) {
	case TS_RPC_CALL_ACCEPTED:
		return MM_RETURN_CODE_SUCCESS;

	case TS_RPC_ERROR_EP_DOES_NOT_EXIT:
		return MM_RETURN_CODE_NOT_SUPPORTED;

	case TS_RPC_ERROR_INVALID_OPCODE:
		return MM_RETURN_CODE_INVALID_PARAMETER;

	case TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED:
		return MM_RETURN_CODE_INVALID_PARAMETER;

	case TS_RPC_ERROR_INVALID_REQ_BODY:
		return MM_RETURN_CODE_INVALID_PARAMETER;

	case TS_RPC_ERROR_INVALID_RESP_BODY:
		return MM_RETURN_CODE_INVALID_PARAMETER;

	case TS_RPC_ERROR_RESOURCE_FAILURE:
		return MM_RETURN_CODE_NOT_SUPPORTED;

	case TS_RPC_ERROR_NOT_READY:
		return MM_RETURN_CODE_NOT_SUPPORTED;

	case TS_RPC_ERROR_INVALID_TRANSACTION:
		return MM_RETURN_CODE_INVALID_PARAMETER;

	case TS_RPC_ERROR_INTERNAL:
		return MM_RETURN_CODE_NOT_SUPPORTED;

	case TS_RPC_ERROR_INVALID_PARAMETER:
		return MM_RETURN_CODE_INVALID_PARAMETER;

	case TS_RPC_ERROR_INTERFACE_DOES_NOT_EXIST:
		return MM_RETURN_CODE_NOT_SUPPORTED;

	default:
		return MM_RETURN_CODE_NOT_SUPPORTED;
	}
}

static int32_t smm_var_receive(struct mm_service_interface *iface,
			       struct mm_service_call_req *mm_req)
{
	SMM_VARIABLE_COMMUNICATE_HEADER *header = NULL;
	struct smm_variable_mm_service *service = iface->context;
	struct call_req rpc_req = { 0 };
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;

	if (mm_req->req_buf.data_len < SMM_VARIABLE_COMMUNICATE_HEADER_SIZE)
		return MM_RETURN_CODE_DENIED;

	header = (SMM_VARIABLE_COMMUNICATE_HEADER *)mm_req->req_buf.data;

	rpc_req.opcode = header->Function;
	rpc_req.req_buf.data = header->Data;
	rpc_req.req_buf.data_len = mm_req->req_buf.data_len - SMM_VARIABLE_COMMUNICATE_HEADER_SIZE;
	rpc_req.req_buf.size = mm_req->req_buf.size - SMM_VARIABLE_COMMUNICATE_HEADER_SIZE;

	rpc_req.resp_buf.data = header->Data;
	rpc_req.resp_buf.data_len = 0;
	rpc_req.resp_buf.size = mm_req->resp_buf.size - SMM_VARIABLE_COMMUNICATE_HEADER_SIZE;

	rpc_status = service->iface->receive(service->iface, &rpc_req);

	header->ReturnStatus = rpc_req.opstatus;

	if (ADD_OVERFLOW(rpc_req.resp_buf.data_len, SMM_VARIABLE_COMMUNICATE_HEADER_SIZE,
			 &mm_req->resp_buf.data_len))
		return MM_RETURN_CODE_NO_MEMORY;

	if (mm_req->resp_buf.data_len > mm_req->resp_buf.size)
		return MM_RETURN_CODE_NO_MEMORY;

	return convert_rpc_status(rpc_status);
}

struct mm_service_interface *smm_variable_mm_service_init(struct smm_variable_mm_service *service,
						       struct rpc_interface *iface)
{
	assert(service != NULL);
	assert(iface != NULL);

	service->iface = iface;
	service->mm_service.context = service;
	service->mm_service.receive = smm_var_receive;

	return &service->mm_service;
}
