/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <string.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/fwu/packed-c/fwu_proto.h>
#include "packedc_fwu_provider_serializer.h"


static rpc_status_t deserialize_open_req(
	const struct call_param_buf *req_buf,
	struct uuid_octets *image_type_uuid)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INVALID_REQ_BODY;
	size_t expected_fixed_len = sizeof(struct ts_fwu_open_in);

	if (expected_fixed_len <= req_buf->data_len) {

		const struct ts_fwu_open_in *recv_msg =
			(const struct ts_fwu_open_in *)req_buf->data;

		memcpy(image_type_uuid->octets,
			recv_msg->image_type_uuid, UUID_OCTETS_LEN);

		rpc_status = TS_RPC_CALL_ACCEPTED;
	}

	return rpc_status;
}

static rpc_status_t serialize_open_resp(
	struct call_param_buf *resp_buf,
	uint32_t handle)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	size_t fixed_len = sizeof(struct ts_fwu_open_out);

	if (fixed_len <= resp_buf->size) {

		struct ts_fwu_open_out *resp_msg =
			(struct ts_fwu_open_out *)resp_buf->data;

		resp_msg->handle = handle;

		resp_buf->data_len = fixed_len;
		rpc_status = TS_RPC_CALL_ACCEPTED;
	}

	return rpc_status;
}

/* Operation: write_stream */
static rpc_status_t deserialize_write_stream_req(
	const struct call_param_buf *req_buf,
	uint32_t *handle,
	size_t *data_len,
	const uint8_t **data)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INVALID_REQ_BODY;
	size_t expected_fixed_len = sizeof(struct ts_fwu_write_stream_in);

	if (expected_fixed_len <= req_buf->data_len) {

		const struct ts_fwu_write_stream_in *recv_msg =
			(const struct ts_fwu_write_stream_in *)req_buf->data;

		*handle = recv_msg->handle;
		*data_len = recv_msg->data_len;
		*data = recv_msg->payload;
		rpc_status = TS_RPC_CALL_ACCEPTED;
	}

	return rpc_status;
}

/* Operation: read_stream */
static rpc_status_t deserialize_read_stream_req(
	const struct call_param_buf *req_buf,
	uint32_t *handle)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INVALID_REQ_BODY;
	size_t expected_fixed_len = sizeof(struct ts_fwu_read_stream_in);

	if (expected_fixed_len <= req_buf->data_len) {

		const struct ts_fwu_read_stream_in *recv_msg =
			(const struct ts_fwu_read_stream_in *)req_buf->data;

		*handle = recv_msg->handle;
		rpc_status = TS_RPC_CALL_ACCEPTED;
	}

	return rpc_status;
}

static void read_stream_resp_payload(
	const struct call_param_buf *resp_buf,
	uint8_t **payload_buf,
	size_t *max_payload)
{
	struct ts_fwu_read_stream_out *resp_msg = (struct ts_fwu_read_stream_out *)resp_buf->data;
	size_t fixed_len = offsetof(struct ts_fwu_read_stream_out, payload);

	*max_payload = 0;
	*payload_buf = resp_msg->payload;

	if (fixed_len < resp_buf->size)
		*max_payload = resp_buf->size - fixed_len;
}

static rpc_status_t serialize_read_stream_resp(
	struct call_param_buf *resp_buf,
	size_t read_bytes,
	size_t total_bytes)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	struct ts_fwu_read_stream_out *resp_msg = (struct ts_fwu_read_stream_out *)resp_buf->data;
	size_t proto_overhead = offsetof(struct ts_fwu_read_stream_out, payload);

	if (read_bytes > (SIZE_MAX - proto_overhead))
		return TS_RPC_ERROR_INVALID_PARAMETER;

	size_t required_len = proto_overhead + read_bytes;

	if (required_len <= resp_buf->size) {

		resp_msg->read_bytes = read_bytes;
		resp_msg->total_bytes = total_bytes;

		resp_buf->data_len = required_len;
		rpc_status = TS_RPC_CALL_ACCEPTED;
	}

	return rpc_status;
}

/* Operation: commit */
static rpc_status_t deserialize_commit_req(
	const struct call_param_buf *req_buf,
	uint32_t *handle,
	bool *accepted,
	size_t *max_atomic_len)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INVALID_REQ_BODY;
	size_t expected_fixed_len = sizeof(struct ts_fwu_commit_in);

	if (expected_fixed_len <= req_buf->data_len) {

		const struct ts_fwu_commit_in *recv_msg =
			(const struct ts_fwu_commit_in *)req_buf->data;

		*handle = recv_msg->handle;
		*accepted = (recv_msg->acceptance_req == 0);
		*max_atomic_len = recv_msg->max_atomic_len;
		rpc_status = TS_RPC_CALL_ACCEPTED;
	}

	return rpc_status;
}

static rpc_status_t serialize_commit_resp(
	struct call_param_buf *resp_buf,
	size_t progress,
	size_t total_work)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	struct ts_fwu_commit_out *resp_msg = (struct ts_fwu_commit_out *)resp_buf->data;

	size_t required_len = sizeof(struct ts_fwu_commit_out);

	if (required_len <= resp_buf->size) {

		resp_msg->progress = progress;
		resp_msg->total_work = total_work;

		resp_buf->data_len = required_len;
		rpc_status = TS_RPC_CALL_ACCEPTED;
	}

	return rpc_status;
}

/* Operation: accept_image */
static rpc_status_t deserialize_accept_req(
	const struct call_param_buf *req_buf,
	struct uuid_octets *image_type_uuid)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INVALID_REQ_BODY;
	size_t expected_fixed_len = sizeof(struct ts_fwu_accept_image_in);

	if (expected_fixed_len <= req_buf->data_len) {

		const struct ts_fwu_accept_image_in *recv_msg =
			(const struct ts_fwu_accept_image_in *)req_buf->data;

		memcpy(image_type_uuid->octets, recv_msg->image_type_uuid, UUID_OCTETS_LEN);
		rpc_status = TS_RPC_CALL_ACCEPTED;
	}

	return rpc_status;
}

/* Singleton method to provide access to the serializer instance */
const struct fwu_provider_serializer *packedc_fwu_provider_serializer_instance(void)
{
	static const struct fwu_provider_serializer instance = {
		deserialize_open_req,
		serialize_open_resp,
		deserialize_write_stream_req,
		deserialize_read_stream_req,
		read_stream_resp_payload,
		serialize_read_stream_resp,
		deserialize_commit_req,
		serialize_commit_resp,
		deserialize_accept_req
	};

	return &instance;
}
