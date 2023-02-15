/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "protocols/service/block_storage/packed-c/opcodes.h"
#include "protocols/rpc/common/packed-c/status.h"
#include "block_storage_provider.h"


/* Service request handlers */
static rpc_status_t get_partition_info_handler(void *context, struct call_req* req);
static rpc_status_t open_handler(void *context, struct call_req* req);
static rpc_status_t close_handler(void *context, struct call_req* req);
static rpc_status_t read_handler(void *context, struct call_req* req);
static rpc_status_t write_handler(void *context, struct call_req* req);
static rpc_status_t erase_handler(void *context, struct call_req* req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{TS_BLOCK_STORAGE_OPCODE_GET_PARTITION_INFO, get_partition_info_handler},
	{TS_BLOCK_STORAGE_OPCODE_OPEN,               open_handler},
	{TS_BLOCK_STORAGE_OPCODE_CLOSE,              close_handler},
	{TS_BLOCK_STORAGE_OPCODE_READ,               read_handler},
	{TS_BLOCK_STORAGE_OPCODE_WRITE,              write_handler},
	{TS_BLOCK_STORAGE_OPCODE_ERASE,              erase_handler}
};

struct rpc_interface *block_storage_provider_init(
	struct block_storage_provider *context,
	struct block_store *block_store)
{
	struct rpc_interface *rpc_interface = NULL;

	if (context) {

		for (size_t encoding = 0; encoding < TS_RPC_ENCODING_LIMIT; ++encoding)
			context->serializers[encoding] = NULL;

		context->block_store = block_store;

		service_provider_init(&context->base_provider, context,
			handler_table, sizeof(handler_table)/sizeof(struct service_handler));

		rpc_interface = service_provider_get_rpc_interface(&context->base_provider);
	}

	return rpc_interface;
}

void block_storage_provider_deinit(
	struct block_storage_provider *context)
{
	(void)context;
}

void block_storage_provider_register_serializer(
	struct block_storage_provider *context,
	unsigned int encoding,
	const struct block_storage_serializer *serializer)
{
	if (encoding < TS_RPC_ENCODING_LIMIT)
		context->serializers[encoding] = serializer;
}

static const struct block_storage_serializer* get_block_storage_serializer(
	struct block_storage_provider *context,
	const struct call_req *req)
{
	const struct block_storage_serializer* serializer = NULL;
	unsigned int encoding = call_req_get_encoding(req);

	if (encoding < TS_RPC_ENCODING_LIMIT) serializer = context->serializers[encoding];

	return serializer;
}

static rpc_status_t get_partition_info_handler(void *context, struct call_req *req)
{
	struct block_storage_provider *this_instance = (struct block_storage_provider*)context;
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;

	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct block_storage_serializer *serializer =
		get_block_storage_serializer(this_instance, req);

	struct uuid_octets partition_guid = {0};

	if (serializer)
		rpc_status = serializer->deserialize_get_partition_info_req(req_buf, &partition_guid);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		struct storage_partition_info partition_info;

		psa_status_t op_status = block_store_get_partition_info(
			this_instance->block_store,
			&partition_guid,
			&partition_info);

		call_req_set_opstatus(req, op_status);

		if (op_status == PSA_SUCCESS) {

			struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
			rpc_status = serializer->serialize_get_partition_info_resp(
				resp_buf,
				&partition_info);
		}
	}

	return rpc_status;
}

static rpc_status_t open_handler(void *context, struct call_req *req)
{
	struct block_storage_provider *this_instance = (struct block_storage_provider*)context;
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;

	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct block_storage_serializer *serializer =
		get_block_storage_serializer(this_instance, req);

	struct uuid_octets partition_guid = {0};

	if (serializer)
		rpc_status = serializer->deserialize_open_req(req_buf, &partition_guid);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		storage_partition_handle_t handle = 0;

		psa_status_t op_status = block_store_open(
			this_instance->block_store,
			req->caller_id,
			&partition_guid,
			&handle);

		call_req_set_opstatus(req, op_status);

		if (op_status == PSA_SUCCESS) {

			struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
			rpc_status = serializer->serialize_open_resp(resp_buf, handle);
		}
	}

	return rpc_status;
}

static rpc_status_t close_handler(void *context, struct call_req *req)
{
	struct block_storage_provider *this_instance = (struct block_storage_provider*)context;
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;

	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct block_storage_serializer *serializer =
		get_block_storage_serializer(this_instance, req);

	storage_partition_handle_t handle = 0;

	if (serializer)
		rpc_status = serializer->deserialize_close_req(req_buf, &handle);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t op_status = block_store_close(
			this_instance->block_store,
			req->caller_id,
			handle);

		call_req_set_opstatus(req, op_status);
	}

	return rpc_status;
}

static rpc_status_t read_handler(void *context, struct call_req *req)
{
	struct block_storage_provider *this_instance = (struct block_storage_provider*)context;
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;

	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct block_storage_serializer *serializer =
		get_block_storage_serializer(this_instance, req);

	storage_partition_handle_t handle = 0;
	uint32_t lba = 0;
	size_t offset = 0;
	size_t len = 0;

	if (serializer)
		rpc_status = serializer->deserialize_read_req(req_buf, &handle, &lba, &offset, &len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		struct call_param_buf *resp_buf = call_req_get_resp_buf(req);

		/* Defend against oversize read length */
		if (len > resp_buf->size)
			len = resp_buf->size;

		psa_status_t op_status = block_store_read(
			this_instance->block_store,
			req->caller_id,
			handle,
			lba,
			offset,
			len,
			(uint8_t*)resp_buf->data,
			&resp_buf->data_len);

		call_req_set_opstatus(req, op_status);
	}

	return rpc_status;
}

static rpc_status_t write_handler(void *context, struct call_req *req)
{
	struct block_storage_provider *this_instance = (struct block_storage_provider*)context;
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;

	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct block_storage_serializer *serializer =
		get_block_storage_serializer(this_instance, req);

	storage_partition_handle_t handle = 0;
	uint32_t lba = 0;
	size_t offset = 0;
	const uint8_t *data = NULL;
	size_t data_len = 0;

	if (serializer)
		rpc_status = serializer->deserialize_write_req(req_buf, &handle, &lba,
			&offset, &data, &data_len);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		size_t num_written = 0;

		psa_status_t op_status = block_store_write(
			this_instance->block_store,
			req->caller_id,
			handle,
			lba,
			offset,
			data,
			data_len,
			&num_written);

		call_req_set_opstatus(req, op_status);

		if (op_status == PSA_SUCCESS) {

			struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
			rpc_status = serializer->serialize_write_resp(resp_buf, num_written);
		}
	}

	return rpc_status;
}

static rpc_status_t erase_handler(void *context, struct call_req *req)
{
	struct block_storage_provider *this_instance = (struct block_storage_provider*)context;
	rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;

	struct call_param_buf *req_buf = call_req_get_req_buf(req);
	const struct block_storage_serializer *serializer =
		get_block_storage_serializer(this_instance, req);

	storage_partition_handle_t handle = 0;
	uint32_t begin_lba = 0;
	size_t num_blocks = 0;

	if (serializer)
		rpc_status = serializer->deserialize_erase_req(req_buf, &handle,
			&begin_lba, &num_blocks);

	if (rpc_status == TS_RPC_CALL_ACCEPTED) {

		psa_status_t op_status = block_store_erase(
			this_instance->block_store,
			req->caller_id,
			handle,
			begin_lba,
			num_blocks);

		call_req_set_opstatus(req, op_status);
	}

	return rpc_status;
}
