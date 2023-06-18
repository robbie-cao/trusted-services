/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <psa/client.h>
#include <psa/sid.h>
#include <trace.h>

#include <protocols/service/capsule_update/capsule_update_proto.h>
#include <protocols/rpc/common/packed-c/status.h>
#include "capsule_update_provider.h"
#include "corstone1000_fmp_service.h"


#define CAPSULE_UPDATE_REQUEST (0x1)
#define KERNEL_STARTED_EVENT   (0x2)

enum corstone1000_ioctl_id_t {
	IOCTL_CORSTONE1000_FWU_FLASH_IMAGES = 0,
	IOCTL_CORSTONE1000_FWU_HOST_ACK,
};

/* Service request handlers */
static rpc_status_t update_capsule_handler(void *context, struct call_req *req);
static rpc_status_t boot_confirmed_handler(void *context, struct call_req *req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{CAPSULE_UPDATE_OPCODE_UPDATE_CAPSULE,			update_capsule_handler},
	{CAPSULE_UPDATE_OPCODE_BOOT_CONFIRMED,			boot_confirmed_handler}
};

struct rpc_interface *capsule_update_provider_init(
	struct capsule_update_provider *context)
{
	struct rpc_interface *rpc_interface = NULL;

	if (context) {

		service_provider_init(
			&context->base_provider,
			context,
			handler_table,
			sizeof(handler_table)/sizeof(struct service_handler));

		rpc_interface = service_provider_get_rpc_interface(&context->base_provider);
	}

	provision_fmp_variables_metadata(context->client.caller);

	return rpc_interface;
}

void capsule_update_provider_deinit(struct capsule_update_provider *context)
{
	(void)context;
}

static rpc_status_t event_handler(uint32_t opcode, struct rpc_caller *caller)
{
	uint32_t ioctl_id;
	rpc_status_t rpc_status = TS_RPC_CALL_ACCEPTED;

	struct psa_invec in_vec[] = {
			{ .base = &ioctl_id, .len = sizeof(ioctl_id) }
	};

	if(!caller) {
		EMSG("event_handler rpc_caller is NULL");
		rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
		return rpc_status;
	}

	IMSG("event handler opcode %x", opcode);
	switch(opcode) {
		case CAPSULE_UPDATE_REQUEST:
		/* Openamp call with IOCTL for firmware update*/
		ioctl_id = IOCTL_CORSTONE1000_FWU_FLASH_IMAGES;
		psa_call(caller,TFM_PLATFORM_SERVICE_HANDLE, TFM_PLATFORM_API_ID_IOCTL,
			in_vec,IOVEC_LEN(in_vec), NULL, 0);
		set_fmp_image_info(caller);
		break;

		case KERNEL_STARTED_EVENT:
		ioctl_id = IOCTL_CORSTONE1000_FWU_HOST_ACK;
		/*openamp call with IOCTL for kernel start*/
		
		psa_call(caller,TFM_PLATFORM_SERVICE_HANDLE, TFM_PLATFORM_API_ID_IOCTL,
			in_vec,IOVEC_LEN(in_vec), NULL, 0);
		set_fmp_image_info(caller);
		break;
		default:
			EMSG("%s unsupported opcode", __func__);
			rpc_status = TS_RPC_ERROR_INVALID_PARAMETER;
			return rpc_status;
	}
	return rpc_status;

}

static rpc_status_t update_capsule_handler(void *context, struct call_req *req)
{
	struct capsule_update_provider *this_instance = (struct capsule_update_provider*)context;
	struct rpc_caller *caller = this_instance->client.caller;
	uint32_t opcode = req->opcode;
	rpc_status_t rpc_status = TS_RPC_ERROR_NOT_READY;

	rpc_status = event_handler(opcode, caller);
	return rpc_status;
}

static rpc_status_t boot_confirmed_handler(void *context, struct call_req *req)
{
	struct capsule_update_provider *this_instance = (struct capsule_update_provider*)context;
	struct rpc_caller *caller = this_instance->client.caller;
	uint32_t opcode = req->opcode;
	rpc_status_t rpc_status = TS_RPC_ERROR_NOT_READY;

	rpc_status = event_handler(opcode, caller);

	return rpc_status;
}
