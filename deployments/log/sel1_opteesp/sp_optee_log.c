// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include "components/rpc/ffarpc/endpoint/sel1_sp/sel1_sp_ffarpc_call_ep.h"
#include "components/service/log/provider/log_provider.h"
#include <ffa.h>
#include <kernel/pseudo_sp.h>
#include <stdio.h>
#include <trace.h>

#define SP_NAME "log.psp"

#define SP_LOG_UUID                                                                                \
	{                                                                                          \
		0xda9dffbd, 0xd590, 0x40ed,                                                        \
		{                                                                                  \
			0x97, 0x5f, 0x19, 0xc6, 0x5a, 0x3d, 0x52, 0xd3                             \
		}                                                                                  \
	}

static void optee_log_puts(const char *str)
{
	trace_ext_puts(str);
}

static struct log_interface optee_log_interface = { .puts = optee_log_puts };

static TEE_Result sp_log_main(uint32_t session_id)
{
	struct sp_session *s = sp_get_session(session_id);
	struct thread_smc_args args = { 0 };
	struct log_provider log_provider = { 0 };
	struct rpc_interface *log_iface = NULL;
	struct ffa_call_ep ffa_call_ep = { 0 };
	uint16_t caller_id = 0;
	uint16_t own_id = 0;

	DMSG("SEL1 log SP init");

	pseudo_sp_ffa(s, &args);

	log_iface = log_provider_init(&log_provider, &optee_log_interface);
	ffa_call_ep_init(&ffa_call_ep, log_iface);

	while (true) {
		caller_id = ((args.a1 >> 16) & 0xffff);
		own_id = (args.a1 & 0xffff);

		ffa_call_ep_receive(&ffa_call_ep, &args, &args);

		args.a0 = FFA_MSG_SEND_DIRECT_RESP_32;
		args.a1 = ((own_id) << 16) | caller_id;
		args.a2 = 0x00;
		args.a3 = 0x00; /* RC = 0 */
		pseudo_sp_ffa(s, &args);
	}

	return TEE_SUCCESS;
}

pseudo_sp_register(.uuid = SP_LOG_UUID, .name = SP_NAME, .invoke_command_entry_point = sp_log_main);
