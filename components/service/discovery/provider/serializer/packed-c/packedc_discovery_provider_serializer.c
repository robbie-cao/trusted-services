/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <string.h>
#include <stdlib.h>
#include <common/tlv/tlv.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/discovery/packed-c/get_service_info.h>
#include "packedc_discovery_provider_serializer.h"

/* Operation: get_service_info */
static rpc_status_t serialize_get_service_info_resp(struct call_param_buf *resp_buf,
	const struct discovery_info *info)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	struct ts_discovery_get_service_info_out resp_msg;
	size_t fixed_len = sizeof(struct ts_discovery_get_service_info_out);

	resp_msg.instance = info->deployment.instance;
	resp_msg.interface_id = info->deployment.interface_id;
	resp_msg.max_req_size = info->deployment.max_req_size;
	resp_msg.supported_encodings = info->supported_encodings;

	if (fixed_len <= resp_buf->size) {

		struct tlv_record out_record;
		struct tlv_iterator resp_iter;
		int encoded_tlv_count = 0;

		memcpy(resp_buf->data, &resp_msg, fixed_len);
		resp_buf->data_len = fixed_len;

		tlv_iterator_begin(&resp_iter,
			(uint8_t*)resp_buf->data + fixed_len,
			resp_buf->size - fixed_len);

		out_record.tag = TS_DISOVERY_GET_SERVICE_INFO_OUT_TAG_NAME_AUTHORITY;
		out_record.length = strlen(info->identity.name_authority) + 1;
		out_record.value = info->identity.name_authority;

		if (tlv_encode(&resp_iter, &out_record)) {

			resp_buf->data_len += tlv_required_space(out_record.length);
			++encoded_tlv_count;
		}

		out_record.tag = TS_DISOVERY_GET_SERVICE_INFO_OUT_TAG_SERVICE_NAME;
		out_record.length = strlen(info->identity.service_name) + 1;
		out_record.value = info->identity.service_name;

		if (tlv_encode(&resp_iter, &out_record)) {

			resp_buf->data_len += tlv_required_space(out_record.length);
			++encoded_tlv_count;
		}

		/* Check that expected TLV records have been encoded */
		if (encoded_tlv_count == 2) rpc_status = TS_RPC_CALL_ACCEPTED;
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
