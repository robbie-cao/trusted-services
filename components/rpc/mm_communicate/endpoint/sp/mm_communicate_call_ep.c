// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#include "components/rpc/mm_communicate/common/mm_communicate_call_args.h"
#include "mm_communicate_call_ep.h"
#include "protocols/common/mm/mm_smc.h"
#include "util.h"
#include <assert.h>
#include <string.h>

bool mm_communicate_call_ep_init(struct mm_communicate_ep *call_ep, uint8_t *comm_buffer,
				 size_t comm_buffer_size)
{
	unsigned int i = 0;

	if (comm_buffer_size < EFI_MM_COMMUNICATE_HEADER_SIZE)
		return false;

	/* Initializing MM communication buffer */
	call_ep->comm_buffer = comm_buffer;
	call_ep->comm_buffer_size = comm_buffer_size;

	/* Initializing service table */
	for (i = 0; i < ARRAY_SIZE(call_ep->service_table); i++) {
		struct mm_service_entry *entry = &call_ep->service_table[i];

		memset(&entry->guid, 0x00, sizeof(entry->guid));
		entry->iface = NULL;
	}

	return true;
}

static int32_t invoke_mm_service(struct mm_communicate_ep *call_ep, uint16_t source_id,
				 struct mm_service_interface *iface,
				 EFI_MM_COMMUNICATE_HEADER *header)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	struct mm_service_call_req call_req = { 0 };
	int32_t result = 0;

	call_req.guid = &header->HeaderGuid;

	/*
	 * The subtraction for size field should be overflow-safe because of the
	 * check in the init funciton.
	 */
	call_req.req_buf.data = header->Data;
	call_req.req_buf.data_len = header->MessageLength;
	call_req.req_buf.size = call_ep->comm_buffer_size - EFI_MM_COMMUNICATE_HEADER_SIZE;

	call_req.resp_buf.data = header->Data;
	call_req.resp_buf.data_len = 0;
	call_req.resp_buf.size = call_ep->comm_buffer_size - EFI_MM_COMMUNICATE_HEADER_SIZE;

	result = iface->receive(iface, &call_req);

	header->MessageLength = call_req.resp_buf.data_len;

	return result;
}

static int32_t handle_mm_communicate(struct mm_communicate_ep *call_ep, uint16_t source_id,
				     uintptr_t buffer_addr, size_t buffer_size)
{
	uintptr_t buffer_arg = 0;
	size_t request_size = 0;
	EFI_MM_COMMUNICATE_HEADER *header = NULL;
	unsigned int i = 0;

	/* Validating call args according to ARM MM spec 3.2.4 */
	if (buffer_addr == 0)
		return MM_RETURN_CODE_INVALID_PARAMETER;

	/* Validating comm buffer contents */
	header = (EFI_MM_COMMUNICATE_HEADER *)call_ep->comm_buffer;
	if (ADD_OVERFLOW(header->MessageLength, EFI_MM_COMMUNICATE_HEADER_SIZE, &request_size))
		return MM_RETURN_CODE_INVALID_PARAMETER;

	if (call_ep->comm_buffer_size < request_size)
		return MM_RETURN_CODE_INVALID_PARAMETER;

	/* Finding iface_id by GUID */
	for (i = 0; i < ARRAY_SIZE(call_ep->service_table); i++) {
		const struct mm_service_entry *entry = &call_ep->service_table[i];

		if (entry->iface != NULL &&
		    memcmp(&header->HeaderGuid, &entry->guid, sizeof(entry->guid)) == 0)
			return invoke_mm_service(call_ep, source_id, entry->iface, header);
	}

	return MM_RETURN_CODE_NOT_SUPPORTED;
}

void mm_communicate_call_ep_attach_service(struct mm_communicate_ep *call_ep, const EFI_GUID *guid,
					   struct mm_service_interface *iface)
{
	unsigned int i = 0;
	struct mm_service_entry *entry = NULL;
	struct mm_service_entry *empty_entry = NULL;

	assert(guid != NULL);
	assert(iface != NULL);

	for (i = 0; i < ARRAY_SIZE(call_ep->service_table); i++) {
		entry = &call_ep->service_table[i];

		if (entry->iface == NULL) {
			empty_entry = entry;
			break;
		}
	}

	assert(empty_entry != NULL);

	memcpy(&empty_entry->guid, guid, sizeof(empty_entry->guid));
	empty_entry->iface = iface;
}

void mm_communicate_call_ep_receive(struct mm_communicate_ep *mm_communicate_call_ep,
				    const struct ffa_direct_msg *req_msg,
				    struct ffa_direct_msg *resp_msg)
{
	int32_t return_value = 0;
	uintptr_t buffer_address = 0;
	size_t buffer_size = 0;

	buffer_address = req_msg->args.args32[MM_COMMUNICATE_CALL_ARGS_COMM_BUFFER_ADDRESS];
	buffer_size = req_msg->args.args32[MM_COMMUNICATE_CALL_ARGS_COMM_BUFFER_SIZE];

	return_value = handle_mm_communicate(mm_communicate_call_ep, req_msg->source_id,
					     buffer_address, buffer_size);

	resp_msg->args.args32[MM_COMMUNICATE_CALL_ARGS_RETURN_ID] = ARM_SVC_ID_SP_EVENT_COMPLETE;
	resp_msg->args.args32[MM_COMMUNICATE_CALL_ARGS_RETURN_CODE] = return_value;
	resp_msg->args.args32[MM_COMMUNICATE_CALL_ARGS_MBZ0] = 0;
	resp_msg->args.args32[MM_COMMUNICATE_CALL_ARGS_MBZ1] = 0;
	resp_msg->args.args32[MM_COMMUNICATE_CALL_ARGS_MBZ2] = 0;
}
