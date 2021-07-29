/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <string.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/discovery/packed-c/get_service_info.h>
#include "packedc_discovery_provider_serializer.h"

/* Operation: get_service_info */
static rpc_status_t serialize_get_service_info_resp(struct call_param_buf *resp_buf,
	size_t max_payload,
	const struct discovery_info *info)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	struct ts_discovery_get_service_info_out resp_msg;
	size_t fixed_len = sizeof(struct ts_discovery_get_service_info_out);

	resp_msg.instance = info->deployment.instance;
	resp_msg.interface_id = info->deployment.interface_id;

	resp_msg.max_payload = max_payload;
	resp_msg.supported_encodings = info->supported_encodings;

	if (fixed_len <= resp_buf->size) {

		memcpy(resp_buf->data, &resp_msg, fixed_len);
		resp_buf->data_len = fixed_len;

		rpc_status = TS_RPC_CALL_ACCEPTED;
	}

	return rpc_status;
}

/* Singleton method to provide access to the serializer instance */
const struct discovery_provider_serializer *packedc_discovery_provider_serializer_instance(void)
{
	static const struct discovery_provider_serializer instance =
	{
		serialize_get_service_info_resp
	};

	return &instance;
}
