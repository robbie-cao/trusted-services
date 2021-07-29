/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <common/tlv/tlv.h>
#include <protocols/service/discovery/packed-c/get_service_info.h>
#include <protocols/service/discovery/packed-c/opcodes.h>
#include <protocols/rpc/common/packed-c/status.h>
#include "discovery_client.h"

psa_status_t discovery_client_get_service_info(
	struct service_client *service_client)
{
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(service_client->caller, &req_buf, 0);

	if (call_handle) {

		uint8_t *resp_buf;
		size_t resp_len;
		int opstatus;

		service_client->rpc_status = rpc_caller_invoke(service_client->caller, call_handle,
			TS_DISCOVERY_OPCODE_GET_SERVICE_INFO, &opstatus, &resp_buf, &resp_len);

		if (service_client->rpc_status == TS_RPC_CALL_ACCEPTED) {

			psa_status = opstatus;

			if (psa_status == PSA_SUCCESS) {

				if (resp_len >= sizeof(struct ts_discovery_get_service_info_out)) {

					struct ts_discovery_get_service_info_out resp_msg;
					memcpy(&resp_msg, resp_buf, sizeof(struct ts_discovery_get_service_info_out));

					service_client->service_info.supported_encodings =
						resp_msg.supported_encodings;

					service_client->service_info.max_payload =
						resp_msg.max_payload;
				}
				else {
					/* Failed to decode response message */
					psa_status = PSA_ERROR_GENERIC_ERROR;
				}
			}
		}

		rpc_caller_end(service_client->caller, call_handle);
	}

	return psa_status;
}
