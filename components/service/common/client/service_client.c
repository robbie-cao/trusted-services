/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <protocols/rpc/common/packed-c/status.h>
#include "service_client.h"

psa_status_t service_client_init(
	struct service_client *context,
	struct rpc_caller *caller)
{
	context->caller = caller;
	context->rpc_status = TS_RPC_CALL_ACCEPTED;

	context->service_access_info.supported_encodings = 0;
	context->service_access_info.max_req_size = 0;

	return PSA_SUCCESS;
}

void service_client_deinit(
	struct service_client *context)
{
	context->caller = NULL;
}

void service_client_set_service_access_info(
	struct service_client *context,
	const struct service_access_info *service_access_info)
{
	context->service_access_info = *service_access_info;
}
