/*
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "remote_fwu_client.h"

#include <cassert>
#include <climits>
#include <cstring>

#include "protocols/rpc/common/packed-c/encoding.h"
#include "protocols/rpc/common/packed-c/status.h"
#include "protocols/service/fwu/packed-c/fwu_proto.h"
#include "protocols/service/fwu/packed-c/opcodes.h"
#include "protocols/service/fwu/packed-c/status.h"
#include "service/discovery/client/discovery_client.h"

remote_fwu_client::remote_fwu_client()
	: fwu_client()
	, m_client()
	, m_rpc_session_handle(NULL)
	, m_service_context(NULL)
{
	open_session();
}

remote_fwu_client::~remote_fwu_client()
{
	close_session();
}

void remote_fwu_client::open_session(void)
{
	struct rpc_caller *caller = NULL;
	int status;

	m_rpc_session_handle = NULL;
	m_service_context = NULL;

	service_locator_init();

	m_service_context = service_locator_query("sn:trustedfirmware.org:fwu:0", &status);

	if (m_service_context) {
		m_rpc_session_handle =
			service_context_open(m_service_context, TS_RPC_ENCODING_PACKED_C, &caller);

		if (m_rpc_session_handle) {
			assert(caller);

			service_client_init(&m_client, caller);
			discovery_client_get_service_info(&m_client);

		} else {
			service_context_relinquish(m_service_context);
			m_service_context = NULL;
		}
	}
}

void remote_fwu_client::close_session(void)
{
	if (m_service_context) {
		service_client_deinit(&m_client);

		if (m_rpc_session_handle) {
			service_context_close(m_service_context, m_rpc_session_handle);
			m_rpc_session_handle = NULL;
		}

		service_context_relinquish(m_service_context);
		m_service_context = NULL;
	}
}

int remote_fwu_client::invoke_no_param(unsigned int opcode)
{
	int fwu_status = FWU_STATUS_NOT_AVAILABLE;
	size_t req_len = 0;
	rpc_call_handle call_handle;
	uint8_t *req_buf;

	if (!m_service_context)
		return fwu_status;

	call_handle = rpc_caller_begin(m_client.caller, &req_buf, req_len);

	if (call_handle) {
		uint8_t *resp_buf;
		size_t resp_len;
		rpc_opstatus_t opstatus;

		m_client.rpc_status = rpc_caller_invoke(m_client.caller, call_handle, opcode,
							&opstatus, &resp_buf, &resp_len);

		if (m_client.rpc_status == TS_RPC_CALL_ACCEPTED)
			fwu_status = opstatus;

		rpc_caller_end(m_client.caller, call_handle);
	}

	return fwu_status;
}

int remote_fwu_client::begin_staging(void)
{
	return invoke_no_param(TS_FWU_OPCODE_BEGIN_STAGING);
}

int remote_fwu_client::end_staging(void)
{
	return invoke_no_param(TS_FWU_OPCODE_END_STAGING);
}

int remote_fwu_client::cancel_staging(void)
{
	return invoke_no_param(TS_FWU_OPCODE_CANCEL_STAGING);
}

int remote_fwu_client::accept(const struct uuid_octets *image_type_uuid)
{
	int fwu_status = FWU_STATUS_NOT_AVAILABLE;
	struct ts_fwu_accept_image_in req_msg = { 0 };
	size_t req_len = sizeof(struct ts_fwu_accept_image_in);

	if (!m_service_context)
		return fwu_status;

	memcpy(req_msg.image_type_uuid, image_type_uuid->octets, OSF_UUID_OCTET_LEN);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_client.caller, &req_buf, req_len);

	if (call_handle) {
		uint8_t *resp_buf;
		size_t resp_len;
		rpc_opstatus_t opstatus;

		memcpy(req_buf, &req_msg, req_len);

		m_client.rpc_status = rpc_caller_invoke(m_client.caller, call_handle,
							TS_FWU_OPCODE_ACCEPT_IMAGE, &opstatus,
							&resp_buf, &resp_len);

		if (m_client.rpc_status == TS_RPC_CALL_ACCEPTED)
			fwu_status = opstatus;

		rpc_caller_end(m_client.caller, call_handle);
	}

	return fwu_status;
}

int remote_fwu_client::select_previous(void)
{
	return invoke_no_param(TS_FWU_OPCODE_SELECT_PREVIOUS);
}

int remote_fwu_client::open(const struct uuid_octets *uuid, uint32_t *handle)
{
	int fwu_status = FWU_STATUS_NOT_AVAILABLE;
	struct ts_fwu_open_in req_msg = { 0 };
	size_t req_len = sizeof(struct ts_fwu_open_in);

	if (!m_service_context)
		return fwu_status;

	memcpy(req_msg.image_type_uuid, uuid->octets, OSF_UUID_OCTET_LEN);

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_client.caller, &req_buf, req_len);

	if (call_handle) {
		uint8_t *resp_buf;
		size_t resp_len;
		rpc_opstatus_t opstatus;

		memcpy(req_buf, &req_msg, req_len);

		m_client.rpc_status = rpc_caller_invoke(m_client.caller, call_handle,
							TS_FWU_OPCODE_OPEN, &opstatus, &resp_buf,
							&resp_len);

		if (m_client.rpc_status == TS_RPC_CALL_ACCEPTED) {
			fwu_status = opstatus;

			if ((fwu_status == FWU_STATUS_SUCCESS) &&
			    (resp_len >= sizeof(struct ts_fwu_open_out))) {
				struct ts_fwu_open_out resp_msg;

				memcpy(&resp_msg, resp_buf, sizeof(struct ts_fwu_open_out));
				*handle = resp_msg.handle;
			}
		}

		rpc_caller_end(m_client.caller, call_handle);
	}

	return fwu_status;
}

int remote_fwu_client::commit(uint32_t handle, bool accepted)
{
	int fwu_status = FWU_STATUS_NOT_AVAILABLE;
	struct ts_fwu_commit_in req_msg = { 0 };
	size_t req_len = sizeof(struct ts_fwu_commit_in);

	if (!m_service_context)
		return fwu_status;

	req_msg.handle = handle;
	req_msg.acceptance_req = (accepted) ? 0 : 1;

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_client.caller, &req_buf, req_len);

	if (call_handle) {
		uint8_t *resp_buf;
		size_t resp_len;
		rpc_opstatus_t opstatus;

		memcpy(req_buf, &req_msg, req_len);

		m_client.rpc_status = rpc_caller_invoke(m_client.caller, call_handle,
							TS_FWU_OPCODE_COMMIT, &opstatus, &resp_buf,
							&resp_len);

		if (m_client.rpc_status == TS_RPC_CALL_ACCEPTED)
			fwu_status = opstatus;

		rpc_caller_end(m_client.caller, call_handle);
	}

	return fwu_status;
}

int remote_fwu_client::write_stream(uint32_t handle, const uint8_t *data, size_t data_len)
{
	size_t proto_overhead = offsetof(ts_fwu_write_stream_in, payload);
	size_t max_payload = (m_client.service_info.max_payload > proto_overhead) ?
				     m_client.service_info.max_payload - proto_overhead :
				     0;

	if (!data || (data_len > (SIZE_MAX - proto_overhead)))
		return FWU_STATUS_OUT_OF_BOUNDS;

	if (!m_service_context)
		return FWU_STATUS_NOT_AVAILABLE;

	if (!max_payload)
		return FWU_STATUS_NOT_AVAILABLE;

	size_t total_written = 0;
	struct ts_fwu_write_stream_in req_msg;

	req_msg.handle = handle;

	while (total_written < data_len) {
		size_t bytes_remaining = data_len - total_written;
		size_t write_len = (bytes_remaining < max_payload) ? bytes_remaining : max_payload;
		size_t req_len = proto_overhead + write_len;

		req_msg.data_len = write_len;

		rpc_call_handle call_handle;
		uint8_t *req_buf;

		call_handle = rpc_caller_begin(m_client.caller, &req_buf, req_len);

		if (call_handle) {
			uint8_t *resp_buf;
			size_t resp_len;
			rpc_opstatus_t opstatus;

			memcpy(req_buf, &req_msg, proto_overhead);
			memcpy(&req_buf[proto_overhead], &data[total_written], write_len);

			total_written += write_len;

			m_client.rpc_status = rpc_caller_invoke(m_client.caller, call_handle,
								TS_FWU_OPCODE_WRITE_STREAM,
								&opstatus, &resp_buf, &resp_len);

			rpc_caller_end(m_client.caller, call_handle);

			if (m_client.rpc_status != TS_RPC_CALL_ACCEPTED)
				return FWU_STATUS_NOT_AVAILABLE;

			if (opstatus != FWU_STATUS_SUCCESS)
				return (int)opstatus;

		} else
			return FWU_STATUS_NOT_AVAILABLE;
	}

	return FWU_STATUS_SUCCESS;
}

int remote_fwu_client::read_stream(uint32_t handle, uint8_t *buf, size_t buf_size, size_t *read_len,
				   size_t *total_len)
{
	int fwu_status = FWU_STATUS_NOT_AVAILABLE;
	struct ts_fwu_read_stream_in req_msg;
	size_t req_len = sizeof(struct ts_fwu_read_stream_in);

	if (!m_service_context)
		return fwu_status;

	req_msg.handle = handle;

	*read_len = 0;
	*total_len = 0;

	rpc_call_handle call_handle;
	uint8_t *req_buf;

	call_handle = rpc_caller_begin(m_client.caller, &req_buf, req_len);

	if (call_handle) {
		uint8_t *resp_buf;
		size_t resp_len;
		rpc_opstatus_t opstatus;

		memcpy(req_buf, &req_msg, req_len);

		m_client.rpc_status = rpc_caller_invoke(m_client.caller, call_handle,
							TS_FWU_OPCODE_READ_STREAM, &opstatus,
							&resp_buf, &resp_len);

		size_t proto_overhead = offsetof(ts_fwu_read_stream_out, payload);

		if (m_client.rpc_status == TS_RPC_CALL_ACCEPTED) {
			fwu_status = opstatus;

			if ((fwu_status == FWU_STATUS_SUCCESS) && (resp_len >= proto_overhead)) {
				const struct ts_fwu_read_stream_out *resp_msg =
					(const struct ts_fwu_read_stream_out *)resp_buf;

				*read_len = resp_msg->read_bytes;
				*total_len = resp_msg->total_bytes;

				if (buf && buf_size) {
					size_t copy_len = (resp_msg->read_bytes <= buf_size) ?
								  resp_msg->read_bytes :
								  buf_size;

					memcpy(buf, resp_msg->payload, copy_len);
				}
			}
		}

		rpc_caller_end(m_client.caller, call_handle);
	}

	return fwu_status;
}
