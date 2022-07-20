// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#include <rpc/ffarpc/endpoint/ffarpc_call_ep.h>
#include "smm_gateway.h"
#include <config/ramstore/config_ramstore.h>
#include "config/interface/config_store.h"
#include <config/loader/sp/sp_config_loader.h>
#include "components/rpc/mm_communicate/endpoint/sp/mm_communicate_call_ep.h"
#include "components/service/smm_variable/frontend/mm_communicate/smm_variable_mm_service.h"
#include "platform/interface/memory_region.h"
#include "protocols/common/mm/mm_smc.h"
#include <ffa_api.h>
#include <sp_api.h>
#include <sp_messaging.h>
#include <sp_rxtx.h>
#include <trace.h>

#define CONFIG_NAME_MM_COMM_BUFFER_REGION	"mm-comm-buffer"

uint16_t own_id = 0; /* !!Needs refactoring as parameter to ffarpc_caller_init */

static int sp_init(uint16_t *own_sp_id);

void __noreturn sp_main(struct ffa_init_info *init_info)
{
	struct memory_region mm_comm_buffer_region = { 0 };
	struct rpc_interface *gateway_iface = NULL;
	struct smm_variable_mm_service smm_var_service = { 0 };
	struct mm_service_interface *smm_var_service_interface = NULL;
	struct mm_communicate_ep mm_communicate_call_ep = { 0 };
	struct ffa_direct_msg req_msg = { 0 };
	struct ffa_direct_msg resp_msg = { 0 };

	static const EFI_GUID smm_variable_guid = SMM_VARIABLE_GUID;

	/* Boot phase */
	if (sp_init(&own_id) != 0) goto fatal_error;

	/* Load any dynamic configuration */
	config_ramstore_init();
	sp_config_load(init_info);

	if (!config_store_query(CONFIG_CLASSIFIER_MEMORY_REGION, CONFIG_NAME_MM_COMM_BUFFER_REGION,
				0, &mm_comm_buffer_region, sizeof(mm_comm_buffer_region))) {
		EMSG(CONFIG_NAME_MM_COMM_BUFFER_REGION " is not set in SP configuration");
		goto fatal_error;
	}

	/* Initialize service layer and associate with RPC endpoint */
	gateway_iface = smm_gateway_create(own_id);

	/* Initialize SMM variable MM service */
	smm_var_service_interface = smm_variable_mm_service_init(&smm_var_service, gateway_iface);

	/* Initialize MM communication layer */
	if (!mm_communicate_call_ep_init(&mm_communicate_call_ep,
					 (void *)mm_comm_buffer_region.base_addr,
					 mm_comm_buffer_region.region_size))
		goto fatal_error;

	/* Attach SMM variable service to MM communication layer */
	mm_communicate_call_ep_attach_service(&mm_communicate_call_ep, &smm_variable_guid,
					      smm_var_service_interface);

	/* End of boot phase */
	ffa_msg_wait(&req_msg);

	while (1) {
		if (FFA_IS_32_BIT_FUNC(req_msg.function_id)) {
			EMSG("MM communicate over 32 bit FF-A messages is not supported");
			ffa_msg_send_direct_resp_32(req_msg.destination_id, req_msg.source_id,
						    MM_RETURN_CODE_NOT_SUPPORTED, 0, 0, 0, 0,
						    &req_msg);
			continue;
		}

		mm_communicate_call_ep_receive(&mm_communicate_call_ep, &req_msg, &resp_msg);

		ffa_msg_send_direct_resp_64(req_msg.destination_id,
					 req_msg.source_id, resp_msg.args.args64[0],
					 resp_msg.args.args64[1], resp_msg.args.args64[2],
					 resp_msg.args.args64[3], resp_msg.args.args64[4],
					 &req_msg);
	}

fatal_error:
	/* SP is not viable */
	EMSG("SMM gateway SP error");
	while (1) {}
}

void sp_interrupt_handler(uint32_t interrupt_id)
{
	(void)interrupt_id;
}

static int sp_init(uint16_t *own_sp_id)
{
	int status = -1;
	ffa_result ffa_res;
	sp_result sp_res;
	static uint8_t tx_buffer[4096] __aligned(4096);
	static uint8_t rx_buffer[4096] __aligned(4096);

	sp_res = sp_rxtx_buffer_map(tx_buffer, rx_buffer, sizeof(rx_buffer));
	if (sp_res == SP_RESULT_OK) {
		ffa_res = ffa_id_get(own_sp_id);
		if (ffa_res == FFA_OK) {
			status = 0;
		}
	}

	return status;
}
