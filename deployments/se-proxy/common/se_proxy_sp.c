// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#include <rpc/ffarpc/endpoint/ffarpc_call_ep.h>
#include <rpc/common/demux/rpc_demux.h>
#include <config/ramstore/config_ramstore.h>
#include <config/loader/sp/sp_config_loader.h>
#include <ffa_api.h>
#include <sp_api.h>
#include <sp_rxtx.h>
#include <trace.h>
#include "service_proxy_factory.h"
#include "../se_proxy_interfaces.h"

uint16_t own_id = 0; /* !!Needs refactoring as parameter to ffarpc_caller_init */


static int sp_init(uint16_t *own_sp_id);

void __noreturn sp_main(struct ffa_init_info *init_info)
{
	struct ffa_call_ep ffarpc_call_ep;
	struct sp_msg req_msg;
	struct rpc_demux rpc_demux;
	struct rpc_interface *rpc_iface;

	/* Boot phase */
	if (sp_init(&own_id) != 0) goto fatal_error;

	config_ramstore_init();
	sp_config_load(init_info);

	rpc_iface = rpc_demux_init(&rpc_demux);
	ffa_call_ep_init(&ffarpc_call_ep, rpc_iface, own_id);

	/* Create service proxies */
	rpc_iface = its_proxy_create();
	rpc_demux_attach(&rpc_demux, SE_PROXY_INTERFACE_ID_ITS, rpc_iface);

	rpc_iface = ps_proxy_create();
	rpc_demux_attach(&rpc_demux, SE_PROXY_INTERFACE_ID_PS, rpc_iface);

	rpc_iface = crypto_proxy_create();
	rpc_demux_attach(&rpc_demux, SE_PROXY_INTERFACE_ID_CRYPTO, rpc_iface);

	rpc_iface = attest_proxy_create();
	rpc_demux_attach(&rpc_demux, SE_PROXY_INTERFACE_ID_ATTEST, rpc_iface);

	/* End of boot phase */
	sp_msg_wait(&req_msg);

	while (1) {

		struct sp_msg resp_msg;

		ffa_call_ep_receive(&ffarpc_call_ep, &req_msg, &resp_msg);

		sp_msg_send_direct_resp(&resp_msg, &req_msg);
	}

fatal_error:
	/* SP is not viable */
	EMSG("SE proxy SP error");
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
