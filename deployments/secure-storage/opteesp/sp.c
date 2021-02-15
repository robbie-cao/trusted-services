/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sp.h"
#include <ffa_api.h>
#include <components/rpc/common/endpoint/rpc_interface.h>
#include <components/rpc/ffarpc/endpoint/ffarpc_call_ep.h>
#include <components/service/secure_storage/backend/secure_flash_store/secure_flash_store.h>
#include <components/service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h>
#include <sp_api.h>
#include <sp_rxtx.h>
#include <trace.h>

uint16_t own_id = 0;
static uint8_t tx_buffer[4096] __aligned(4096);
static uint8_t rx_buffer[4096] __aligned(4096);

void sp_main(struct ffa_init_info *init_info)
{
	ffa_result ffa_res;
	sp_result sp_res;
	struct rpc_interface *secure_storage_iface;
	struct ffa_call_ep ffa_call_ep;
	struct ffa_direct_msg req_msg;
	struct ffa_direct_msg resp_msg;
	struct secure_storage_provider secure_storage_provider;
	struct storage_backend *storage_backend;

	/* Boot */
	(void) init_info;

	ffa_res = ffa_id_get(&own_id);
	if (ffa_res != FFA_OK) {
		EMSG("id get error: %d", ffa_res);
	}

	sp_res = sp_rxtx_buffer_map(tx_buffer, rx_buffer, sizeof(rx_buffer));
	if (sp_res != SP_RESULT_OK) {
		EMSG("rxtx map error: %d", sp_res);
	}

	storage_backend = sfs_init();
	secure_storage_iface = secure_storage_provider_init(&secure_storage_provider, storage_backend);
	ffa_call_ep_init(&ffa_call_ep, secure_storage_iface);

	/* End of boot phase */
	ffa_msg_wait(&req_msg);

	while (1) {
		if (req_msg.function_id == FFA_MSG_SEND_DIRECT_REQ_32) {
			ffa_call_ep_receive(&ffa_call_ep, &req_msg, &resp_msg);

			ffa_msg_send_direct_resp(req_msg.destination_id,
					req_msg.source_id, resp_msg.args[0], resp_msg.args[1],
					resp_msg.args[2], resp_msg.args[3], resp_msg.args[4],
					&req_msg);
		}
	}
}

void sp_interrupt_handler(uint32_t interrupt_id)
{
	(void)interrupt_id;
}
