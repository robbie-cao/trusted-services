// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include <rpc/ffarpc/caller/sp/ffarpc_caller.h>
#include <rpc/ffarpc/endpoint/ffarpc_call_ep.h>
#include <service/test_runner/provider/test_runner_provider.h>
#include <service/test_runner/provider/serializer/packed-c/packedc_test_runner_provider_serializer.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <config/ramstore/config_ramstore.h>
#include <config/loader/sp/sp_config_loader.h>
#include <ffa_api.h>
#include <sp_api.h>
#include <sp_rxtx.h>
#include <trace.h>
#include "env_test_tests.h"


uint16_t own_id = 0; /* !!Needs refactoring as parameter to ffarpc_caller_init */


static int sp_init(uint16_t *own_sp_id);

void __noreturn sp_main(struct ffa_init_info *init_info)
{
	struct test_runner_provider test_runner_provider;
	struct ffa_call_ep ffarpc_call_ep;
	struct rpc_interface *test_runner_iface;
	struct ffarpc_caller ffarpc_caller;
	struct ffa_direct_msg req_msg;

	/* Boot */
	if (sp_init(&own_id) != 0) goto fatal_error;

	config_ramstore_init();
	sp_config_load(init_info);

	/* Initialize the test_runner service */
	test_runner_iface = test_runner_provider_init(&test_runner_provider);

	test_runner_provider_register_serializer(&test_runner_provider,
			TS_RPC_ENCODING_PACKED_C, packedc_test_runner_provider_serializer_instance());

	env_test_register_tests(&test_runner_provider);

	ffa_call_ep_init(&ffarpc_call_ep, test_runner_iface);

 	/* End of boot phase */
	ffa_msg_wait(&req_msg);

	while (1) {
		if (req_msg.function_id == FFA_MSG_SEND_DIRECT_REQ_32) {

			struct ffa_direct_msg resp_msg;

			ffa_call_ep_receive(&ffarpc_call_ep, &req_msg, &resp_msg);

			ffa_msg_send_direct_resp(req_msg.destination_id,
					req_msg.source_id, resp_msg.args[0], resp_msg.args[1],
					resp_msg.args[2], resp_msg.args[3], resp_msg.args[4],
					&req_msg);
		}
	}

fatal_error:
	/* SP is not viable */
	EMSG("environment-test SP error");
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
