// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#include "rpc/ffarpc/endpoint/ffarpc_call_ep.h"
#include "rpc/common/demux/rpc_demux.h"
#include "config/ramstore/config_ramstore.h"
#include "config/loader/sp/sp_config_loader.h"
#include "sp_api.h"
#include "sp_discovery.h"
#include "sp_rxtx.h"
#include "trace.h"
#include "deployments/se-proxy/infra/service_proxy_factory.h"
#include "deployments/se-proxy/se_proxy_interfaces.h"

static bool sp_init(uint16_t *own_sp_id);

void __noreturn sp_main(struct ffa_init_info *init_info)
{
	struct ffa_call_ep ffarpc_call_ep = { 0 };
	struct sp_msg req_msg = { 0 };
	struct sp_msg resp_msg = { 0 };
	struct rpc_demux rpc_demux = { 0 };
	struct rpc_interface *rpc_iface = NULL;
	uint16_t own_id = 0;
	sp_result result = SP_RESULT_INTERNAL_ERROR;

	/* Boot phase */
	if (!sp_init(&own_id)) {
		EMSG("Failed to init SP");
		goto fatal_error;
	}

	config_ramstore_init();

	if (!sp_config_load(init_info)) {
		EMSG("Failed to load SP config");
		goto fatal_error;
	}

	rpc_iface = rpc_demux_init(&rpc_demux);
	if (!rpc_iface) {
		EMSG("Failed to initialize RPC demux");
		goto fatal_error;
	}

	ffa_call_ep_init(&ffarpc_call_ep, rpc_iface, own_id);

	/* Create service proxies */
	rpc_iface = its_proxy_create();
	if (!rpc_iface) {
		EMSG("Failed to create ITS proxy");
		goto fatal_error;
	}

	rpc_demux_attach(&rpc_demux, SE_PROXY_INTERFACE_ID_ITS, rpc_iface);

	rpc_iface = ps_proxy_create();
	if (!rpc_iface) {
		EMSG("Failed to create PS proxy");
		goto fatal_error;
	}
	rpc_demux_attach(&rpc_demux, SE_PROXY_INTERFACE_ID_PS, rpc_iface);

	rpc_iface = crypto_proxy_create();
	if (!rpc_iface) {
		EMSG("Failed to create Crypto proxy");
		goto fatal_error;
	}
	rpc_demux_attach(&rpc_demux, SE_PROXY_INTERFACE_ID_CRYPTO, rpc_iface);

	rpc_iface = attest_proxy_create();
	if (!rpc_iface) {
		EMSG("Failed to create Attestation proxy");
		goto fatal_error;
	}
	rpc_demux_attach(&rpc_demux, SE_PROXY_INTERFACE_ID_ATTEST, rpc_iface);

	/* End of boot phase */
	result = sp_msg_wait(&req_msg);
	if (result != SP_RESULT_OK) {
		EMSG("Failed to send message wait %d", result);
		goto fatal_error;
	}

	while (1) {
		ffa_call_ep_receive(&ffarpc_call_ep, &req_msg, &resp_msg);

		result = sp_msg_send_direct_resp(&resp_msg, &req_msg);
		if (result != SP_RESULT_OK) {
			EMSG("Failed to send direct response %d", result);
			result = sp_msg_wait(&req_msg);
			if (result != SP_RESULT_OK) {
				EMSG("Failed to send message wait %d", result);
				goto fatal_error;
			}
		}
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

static bool sp_init(uint16_t *own_id)
{
	sp_result sp_res = SP_RESULT_INTERNAL_ERROR;
	static uint8_t tx_buffer[4096] __aligned(4096);
	static uint8_t rx_buffer[4096] __aligned(4096);

	sp_res = sp_rxtx_buffer_map(tx_buffer, rx_buffer, sizeof(rx_buffer));
	if (sp_res != SP_RESULT_OK) {
		EMSG("Failed to map RXTX buffers: %d", sp_res);
		return false;
	}

	sp_res = sp_discovery_own_id_get(own_id);
	if (sp_res != SP_RESULT_OK) {
		EMSG("Failed to query own ID: %d", sp_res);
		return false;
	}

	return true;
}
