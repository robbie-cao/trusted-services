/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include "../ffarpc_caller.h"
#include "protocols/rpc/common/packed-c/status.h"
#include "mock_ffa_api.h"
#include "mock_sp_memory_management.h"
#include "mock_sp_messaging.h"
#include "mock_sp_rxtx.h"
#include <string.h>

extern uint8_t shared_buffer[4096];

static const uint8_t uuid[16] = {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
	0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0
};

static const struct ffa_partition_information partiton_info[5] = {
	{.partition_id = 0x8001, .execution_context_count = 1, .partition_properties = 0},
	{.partition_id = 0x8002, .execution_context_count = 1, .partition_properties = 0},
	{.partition_id = 0x8003, .execution_context_count = 1, .partition_properties = 0},
	{.partition_id = 0x8004, .execution_context_count = 1, .partition_properties = 0},
	{.partition_id = 0x8005, .execution_context_count = 1, .partition_properties = 0},
};

TEST_GROUP(ffarpc_caller) {
	TEST_SETUP()
	{
		own_id = 0x0123;
		rpc_caller_instance = ffarpc_caller_init(&caller, own_id);
		memset(&req, 0x00, sizeof(req));
		memset(&resp, 0x00, sizeof(resp));
		memset(&desc, 0x00, sizeof(desc));
		memset(&acc_desc, 0x00, sizeof(acc_desc));
		memset(&region, 0x00, sizeof(region));
	}

	TEST_TEARDOWN()
	{
		ffarpc_caller_deinit(&caller);
		mock().checkExpectations();
		mock().clear();
	}

	void setup_mem_share_descriptors(uint16_t sender_id, uint16_t dest_id)
	{
		desc.sender_id = sender_id;
		desc.memory_type = sp_memory_type_normal_memory;
		desc.mem_region_attr.normal_memory.cacheability = sp_cacheability_write_back;
		desc.mem_region_attr.normal_memory.shareability = sp_shareability_inner_shareable;

		acc_desc.data_access = sp_data_access_read_write;
		acc_desc.instruction_access = sp_instruction_access_not_specified;
		acc_desc.receiver_id = dest_id;

		region.address = shared_buffer;
		region.page_count = 1;
	}

	void expect_mem_share_msg(uint16_t src_id, uint16_t dst_id, uint64_t handle,
				  size_t buffer_size, rpc_status_t rpc_status, sp_result res)
	{
		expect_mem_handler_msg(0, src_id, dst_id, handle, buffer_size, rpc_status, res);
	}

	void expect_mem_unshare_msg(uint16_t src_id, uint16_t dst_id, uint64_t handle,
				    rpc_status_t rpc_status, sp_result res)
	{
		expect_mem_handler_msg(1, src_id, dst_id, handle, 0, rpc_status, res);
	}

	void expect_mem_handler_msg(uint16_t opcode, uint16_t src_id, uint16_t dst_id,
				    uint64_t handle, size_t buffer_size,
				    rpc_status_t rpc_status, sp_result res)
	{
		req.source_id = src_id;
		req.destination_id = dst_id;
		req.args.args32[0] = 0x1000 << 16 | opcode;
		req.args.args32[1] = (handle & UINT32_MAX);
		req.args.args32[2] = ((handle >> 32) & UINT32_MAX);
		req.args.args32[3] = buffer_size;

		resp.source_id = dst_id;
		resp.destination_id = src_id;
		resp.args.args32[0] = 0x1000 << 16 | opcode;
		resp.args.args32[2] = rpc_status;

		expect_sp_msg_send_direct_req(&req, &resp, res);
	}

	uint16_t own_id;
	struct ffarpc_caller caller;
	rpc_caller *rpc_caller_instance;
	struct sp_msg req;
	struct sp_msg resp;
	struct sp_memory_descriptor desc;
	struct sp_memory_access_descriptor acc_desc;
	struct sp_memory_region region;
};

TEST(ffarpc_caller, discover_invalid_arguments)
{
	uint16_t sp_ids[5] = {0};

	UNSIGNED_LONGS_EQUAL(0, ffarpc_caller_discover(NULL, sp_ids, 5));
	UNSIGNED_LONGS_EQUAL(0, ffarpc_caller_discover(uuid, NULL, 5));
	UNSIGNED_LONGS_EQUAL(0, ffarpc_caller_discover(uuid, sp_ids, 0));
}

TEST(ffarpc_caller, discover_partition_info_get_fail)
{
	uint32_t count = 0;
	uint16_t sp_ids[5] = {0};

	expect_ffa_partition_info_get((struct ffa_uuid *)uuid, &count, FFA_BUSY);
	UNSIGNED_LONGS_EQUAL(0, ffarpc_caller_discover(uuid, sp_ids, 5));
}

TEST(ffarpc_caller, discover_rx_get_fail)
{
	uint32_t count = 5;
	uint16_t sp_ids[5] = {0};
	const void *buffer = NULL;
	size_t size = 0;

	expect_ffa_partition_info_get((struct ffa_uuid *)uuid, &count, FFA_OK);
	expect_sp_rxtx_buffer_rx_get(&buffer, &size, SP_RESULT_INTERNAL_ERROR);
	UNSIGNED_LONGS_EQUAL(0, ffarpc_caller_discover(uuid, sp_ids, 5));
}

TEST(ffarpc_caller, discover_rx_release_fail)
{
	uint32_t count = 5;
	uint16_t sp_ids[5] = {0};
	const void *buffer_ptr = partiton_info;
	size_t size = sizeof(partiton_info);

	expect_ffa_partition_info_get((struct ffa_uuid *)uuid, &count, FFA_OK);
	expect_sp_rxtx_buffer_rx_get(&buffer_ptr, &size, SP_RESULT_OK);
	expect_ffa_rx_release(FFA_BUSY);
	UNSIGNED_LONGS_EQUAL(0, ffarpc_caller_discover(uuid, sp_ids, 5));
}

TEST(ffarpc_caller, discover)
{
	uint32_t count = 5;
	uint16_t sp_ids[5] = {0};
	const void *buffer_ptr = partiton_info;
	size_t size = sizeof(partiton_info);
	uint16_t expected_sp_ids[5] = {0x8001, 0x8002, 0x8003, 0x8004, 0x8005};

	expect_ffa_partition_info_get((struct ffa_uuid *)uuid, &count, FFA_OK);
	expect_sp_rxtx_buffer_rx_get(&buffer_ptr, &size, SP_RESULT_OK);
	expect_ffa_rx_release(FFA_OK);
	UNSIGNED_LONGS_EQUAL(count, ffarpc_caller_discover(uuid, sp_ids, count));
	MEMCMP_EQUAL(expected_sp_ids, sp_ids, sizeof(expected_sp_ids));
}

TEST(ffarpc_caller, discover_less_max_cnt)
{	struct sp_msg req = {0};
	struct sp_msg resp = {0};
	struct sp_memory_descriptor desc = { 0 };
	struct sp_memory_access_descriptor acc_desc = { 0 };
	struct sp_memory_region region = { 0 };
	uint32_t count = 5;
	uint16_t sp_ids[2] = {0};
	const void *buffer_ptr = partiton_info;
	size_t size = sizeof(partiton_info);
	uint16_t expected_sp_ids[2] = {0x8001, 0x8002};

	expect_ffa_partition_info_get((struct ffa_uuid *)uuid, &count, FFA_OK);
	expect_sp_rxtx_buffer_rx_get(&buffer_ptr, &size, SP_RESULT_OK);
	expect_ffa_rx_release(FFA_OK);
	UNSIGNED_LONGS_EQUAL(count, ffarpc_caller_discover(uuid, sp_ids, 2));
	MEMCMP_EQUAL(expected_sp_ids, sp_ids, sizeof(expected_sp_ids));
}

TEST(ffarpc_caller, discover_more_max_cnt)
{
	uint32_t count = 2;
	uint16_t sp_ids[5] = {0};
	const void *buffer_ptr = partiton_info;
	size_t size = sizeof(partiton_info);
	uint16_t expected_sp_ids[5] = {0x8001, 0x8002, 0, 0, 0};

	expect_ffa_partition_info_get((struct ffa_uuid *)uuid, &count, FFA_OK);
	expect_sp_rxtx_buffer_rx_get(&buffer_ptr, &size, SP_RESULT_OK);
	expect_ffa_rx_release(FFA_OK);
	UNSIGNED_LONGS_EQUAL(count, ffarpc_caller_discover(uuid, sp_ids, 5));
	MEMCMP_EQUAL(expected_sp_ids, sp_ids, sizeof(expected_sp_ids));
}

TEST(ffarpc_caller, open_already_opened)
{
	caller.dest_partition_id = 1;
	LONGS_EQUAL(-1, ffarpc_caller_open(&caller, 0, 0));
}

TEST(ffarpc_caller, open_invalid_mem_size)
{
	caller.shared_mem_required_size = (size_t)UINT32_MAX + 1;
	LONGS_EQUAL(-1, ffarpc_caller_open(&caller, 0, 0));
}

TEST(ffarpc_caller, open_share_fail)
{
	uint16_t dest_id = 0x1234;
	uint64_t handle = 0x56789abcdef01234ULL;

	setup_mem_share_descriptors(own_id, dest_id);

	expect_sp_memory_share(&desc, &acc_desc, 1, &region, 1, &handle,
			       SP_RESULT_INVALID_PARAMETERS);
	LONGS_EQUAL(-1, ffarpc_caller_open(&caller, dest_id, 0));
}

TEST(ffarpc_caller, open_send_direct_req_fail)
{
	uint16_t dest_id = 0x1234;
	uint64_t handle = 0x56789abcdef01234ULL;

	setup_mem_share_descriptors(own_id, dest_id);

	expect_sp_memory_share(&desc, &acc_desc, 1, &region, 1, &handle, SP_RESULT_OK);
	expect_mem_share_msg(own_id, dest_id, handle, sizeof(shared_buffer),
			     TS_RPC_ERROR_INTERNAL, SP_RESULT_INTERNAL_ERROR);
	LONGS_EQUAL(-1, ffarpc_caller_open(&caller, dest_id, 0));
}

TEST(ffarpc_caller, open_send_direct_req_rpc_status_fail)
{
	uint16_t dest_id = 0x1234;
	uint64_t handle = 0x56789abcdef01234ULL;

	setup_mem_share_descriptors(own_id, dest_id);

	expect_sp_memory_share(&desc, &acc_desc, 1, &region, 1, &handle, SP_RESULT_OK);
	expect_mem_share_msg(own_id, dest_id, handle, sizeof(shared_buffer),
			     TS_RPC_ERROR_INVALID_PARAMETER, SP_RESULT_OK);
	expect_sp_memory_reclaim(handle, 0, SP_RESULT_OK);
	LONGS_EQUAL(-1, ffarpc_caller_open(&caller, dest_id, 0));
}

TEST(ffarpc_caller, open_send_direct_req_rpc_status_fail_reclaim_fail)
{
	uint16_t dest_id = 0x1234;
	uint64_t handle = 0x56789abcdef01234ULL;

	setup_mem_share_descriptors(own_id, dest_id);

	expect_sp_memory_share(&desc, &acc_desc, 1, &region, 1, &handle, SP_RESULT_OK);
	expect_mem_share_msg(own_id, dest_id, handle, sizeof(shared_buffer),
			     TS_RPC_ERROR_INVALID_PARAMETER, SP_RESULT_OK);
	expect_sp_memory_reclaim(handle, 0, SP_RESULT_INVALID_PARAMETERS);
	LONGS_EQUAL(-1, ffarpc_caller_open(&caller, dest_id, 0));
}

TEST(ffarpc_caller, open_success)
{
	uint16_t dest_id = 0x1234;
	uint64_t handle = 0x56789abcdef01234ULL;

	setup_mem_share_descriptors(own_id, dest_id);

	expect_sp_memory_share(&desc, &acc_desc, 1, &region, 1, &handle, SP_RESULT_OK);
	expect_mem_share_msg(own_id, dest_id, handle, sizeof(shared_buffer),
			     TS_RPC_CALL_ACCEPTED, SP_RESULT_OK);
	LONGS_EQUAL(0, ffarpc_caller_open(&caller, dest_id, 0));
}

TEST(ffarpc_caller, close_not_opened)
{
	LONGS_EQUAL(-1, ffarpc_caller_close(&caller));
}

TEST(ffarpc_caller, close_send_direct_msg_fail)
{
	caller.shared_mem_handle = 0x56789abcdef01234ULL;
	caller.dest_partition_id = 0x1234;

	expect_mem_unshare_msg(own_id, caller.dest_partition_id, caller.shared_mem_handle,
			       TS_RPC_ERROR_INTERNAL, SP_RESULT_INVALID_PARAMETERS);
	LONGS_EQUAL(-1, ffarpc_caller_close(&caller));
}

TEST(ffarpc_caller, close_mem_reclaim_fail)
{
	caller.shared_mem_handle = 0x56789abcdef01234ULL;
	caller.dest_partition_id = 0x1234;

	expect_mem_unshare_msg(own_id, caller.dest_partition_id, caller.shared_mem_handle,
			       TS_RPC_CALL_ACCEPTED, SP_RESULT_OK);
	expect_sp_memory_reclaim(caller.shared_mem_handle, 0, SP_RESULT_INVALID_PARAMETERS);
	LONGS_EQUAL(-1, ffarpc_caller_close(&caller));
}

TEST(ffarpc_caller, close_endpoint_and_mem_reclaim_fail)
{
	caller.shared_mem_handle = 0x56789abcdef01234ULL;
	caller.dest_partition_id = 0x1234;

	expect_mem_unshare_msg(own_id, caller.dest_partition_id, caller.shared_mem_handle,
			       TS_RPC_ERROR_ACCESS_DENIED, SP_RESULT_OK);
	expect_sp_memory_reclaim(caller.shared_mem_handle, 0, SP_RESULT_INVALID_PARAMETERS);
	LONGS_EQUAL(-1, ffarpc_caller_close(&caller));
}

TEST(ffarpc_caller, close_success)
{
	caller.shared_mem_handle = 0x56789abcdef01234ULL;
	caller.dest_partition_id = 0x1234;

	expect_mem_unshare_msg(own_id, caller.dest_partition_id, caller.shared_mem_handle,
			       TS_RPC_CALL_ACCEPTED, SP_RESULT_OK);
	expect_sp_memory_reclaim(caller.shared_mem_handle, 0, SP_RESULT_OK);
	LONGS_EQUAL(0, ffarpc_caller_close(&caller));
}

TEST(ffarpc_caller, begin_null_context)
{
	uint8_t *buffer = NULL;

	rpc_caller_instance->context = NULL;

	POINTERS_EQUAL(NULL,  rpc_caller_begin(rpc_caller_instance, &buffer, 0));
}

TEST(ffarpc_caller, begin_null_buffer)
{
	POINTERS_EQUAL(NULL, rpc_caller_begin(rpc_caller_instance, NULL, 0));
}

TEST(ffarpc_caller, begin_transaction_in_progress)
{
	uint8_t *buffer = NULL;

	caller.is_call_transaction_in_progess = true;
	POINTERS_EQUAL(NULL, rpc_caller_begin(rpc_caller_instance, &buffer, 0));
}

TEST(ffarpc_caller, begin_too_large_req)
{
	uint8_t *buffer = NULL;

	POINTERS_EQUAL(NULL, rpc_caller_begin(rpc_caller_instance, &buffer,
					      sizeof(shared_buffer) + 1));
}

TEST(ffarpc_caller, begin_not_opened)
{
	uint8_t *buffer = NULL;

	rpc_call_handle handle = rpc_caller_begin(rpc_caller_instance, &buffer,
						  0);
	POINTERS_EQUAL(NULL, handle);
}

TEST(ffarpc_caller, begin_with_buffer)
{
	uint8_t *buffer = NULL;

	caller.dest_partition_id = 1;
	rpc_call_handle handle = rpc_caller_begin(rpc_caller_instance, &buffer,
						  sizeof(shared_buffer));
	CHECK_TRUE(handle != NULL);
	POINTERS_EQUAL(shared_buffer, buffer);
}

TEST(ffarpc_caller, begin_without_buffer)
{
	uint8_t *buffer = NULL;

	caller.dest_partition_id = 1;
	rpc_call_handle handle = rpc_caller_begin(rpc_caller_instance, &buffer,
						  0);
	CHECK_TRUE(handle != NULL);
	POINTERS_EQUAL(NULL, buffer);
}

TEST(ffarpc_caller, invoke_null_context)
{
	rpc_call_handle handle = &caller;
	rpc_opstatus_t opstatus = 0;
	uint8_t *resp_buf = NULL;
	size_t resp_len = 0;

	rpc_caller_instance->context = NULL;
	LONGS_EQUAL(TS_RPC_ERROR_INVALID_PARAMETER,
		    rpc_caller_invoke(rpc_caller_instance, NULL, 0, &opstatus,
				      &resp_buf, &resp_len));
}

TEST(ffarpc_caller, invoke_handle_context_diff)
{
	rpc_opstatus_t opstatus = 0;
	uint8_t *resp_buf = NULL;
	size_t resp_len = 0;

	LONGS_EQUAL(TS_RPC_ERROR_INVALID_PARAMETER,
		    rpc_caller_invoke(rpc_caller_instance, NULL, 0, &opstatus,
				      &resp_buf, &resp_len));
}

TEST(ffarpc_caller, invoke_opstatus_null)
{
	rpc_call_handle handle = &caller;
	uint8_t *resp_buf = NULL;
	size_t resp_len = 0;

	LONGS_EQUAL(TS_RPC_ERROR_INVALID_PARAMETER,
		    rpc_caller_invoke(rpc_caller_instance, handle, 0, NULL,
				      &resp_buf, &resp_len));
}

TEST(ffarpc_caller, invoke_resp_buf_null)
{
	rpc_call_handle handle = &caller;
	rpc_opstatus_t opstatus = 0;
	size_t resp_len = 0;

	LONGS_EQUAL(TS_RPC_ERROR_INVALID_PARAMETER,
		    rpc_caller_invoke(rpc_caller_instance, handle, 0,
				      &opstatus, NULL, &resp_len));
}

TEST(ffarpc_caller, invoke_resp_len_null)
{
	rpc_call_handle handle = &caller;
	rpc_opstatus_t opstatus = 0;
	uint8_t *resp_buf = NULL;

	LONGS_EQUAL(TS_RPC_ERROR_INVALID_PARAMETER,
		    rpc_caller_invoke(rpc_caller_instance, handle, 0,
				      &opstatus, &resp_buf, NULL));
}

TEST(ffarpc_caller, invoke_resp_no_begin)
{
	rpc_call_handle handle = &caller;
	rpc_opstatus_t opstatus = 0;
	uint8_t *resp_buf = NULL;
	size_t resp_len = 0;

	LONGS_EQUAL(TS_RPC_ERROR_NOT_READY,
		    rpc_caller_invoke(rpc_caller_instance, handle, 0,
				      &opstatus, &resp_buf, &resp_len));
}

TEST(ffarpc_caller, invoke_resp_long_req)
{
	rpc_call_handle handle = &caller;
	rpc_opstatus_t opstatus = 0;
	uint8_t *resp_buf = NULL;
	size_t resp_len = 0;

	caller.is_call_transaction_in_progess = true;
	caller.req_len = (size_t)UINT32_MAX + 1;
	LONGS_EQUAL(TS_RPC_ERROR_INTERNAL,
		    rpc_caller_invoke(rpc_caller_instance, handle, 0,
				      &opstatus, &resp_buf, &resp_len));
}

TEST(ffarpc_caller, invoke_send_direct_req_fail)
{
	rpc_call_handle handle = &caller;
	uint16_t opcode = 0xfedc;
	rpc_opstatus_t opstatus = 0;
	uint8_t *resp_buf = NULL;
	size_t resp_len = 0;

	caller.is_call_transaction_in_progess = true;
	caller.shared_mem_handle = 0x56789abcdef01234ULL;
	caller.dest_partition_id = 0x1234;
	caller.req_len = 0x3456789a;
	caller.rpc_caller.encoding = 0xabcdef12;

	req.source_id = own_id;
	req.destination_id = caller.dest_partition_id;
	req.args.args32[0] = opcode;
	req.args.args32[1] = caller.req_len;
	req.args.args32[2] = 0;
	req.args.args32[3] = caller.rpc_caller.encoding;

	expect_sp_msg_send_direct_req(&req, &resp, SP_RESULT_INTERNAL_ERROR);
	LONGS_EQUAL(TS_RPC_ERROR_INTERNAL,
		    rpc_caller_invoke(rpc_caller_instance, handle, opcode,
				      &opstatus, &resp_buf, &resp_len));
}

TEST(ffarpc_caller, invoke_invalid_resp_len)
{
	rpc_call_handle handle = &caller;
	uint16_t opcode = 0xfedc;
	rpc_opstatus_t opstatus = 0;
	uint8_t *resp_buf = NULL;
	size_t resp_len = 0;

	caller.is_call_transaction_in_progess = true;
	caller.shared_mem_handle = 0x56789abcdef01234ULL;
	caller.dest_partition_id = 0x1234;
	caller.req_len = 0x3456789a;
	caller.rpc_caller.encoding = 0xabcdef12;

	req.source_id = own_id;
	req.destination_id = caller.dest_partition_id;
	req.args.args32[0] = opcode;
	req.args.args32[1] = caller.req_len;
	req.args.args32[2] = 0;
	req.args.args32[3] = caller.rpc_caller.encoding;

	resp.source_id = caller.dest_partition_id;
	resp.destination_id = own_id;
	resp.args.args32[0] = opcode;
	resp.args.args32[1] = sizeof(shared_buffer) + 1;
	resp.args.args32[2] = TS_RPC_CALL_ACCEPTED;
	resp.args.args32[3] = 0;

	expect_sp_msg_send_direct_req(&req, &resp, SP_RESULT_OK);
	LONGS_EQUAL(TS_RPC_ERROR_INTERNAL,
		    rpc_caller_invoke(rpc_caller_instance, handle, opcode,
				      &opstatus, &resp_buf, &resp_len));
}

TEST(ffarpc_caller, invoke_with_response)
{
	rpc_call_handle handle = &caller;
	uint16_t opcode = 0xfedc;
	rpc_opstatus_t opstatus = 0;
	uint8_t *resp_buf = NULL;
	size_t resp_len = 0;

	caller.is_call_transaction_in_progess = true;
	caller.shared_mem_handle = 0x56789abcdef01234ULL;
	caller.dest_partition_id = 0x1234;
	caller.req_len = 0x3456789a;
	caller.rpc_caller.encoding = 0xabcdef12;

	req.source_id = own_id;
	req.destination_id = caller.dest_partition_id;
	req.args.args32[0] = opcode;
	req.args.args32[1] = caller.req_len;
	req.args.args32[2] = 0;
	req.args.args32[3] = caller.rpc_caller.encoding;

	resp.source_id = caller.dest_partition_id;
	resp.destination_id = own_id;
	resp.args.args32[0] = opcode;
	resp.args.args32[1] = sizeof(shared_buffer);
	resp.args.args32[2] = TS_RPC_CALL_ACCEPTED;
	resp.args.args32[3] = 0;

	expect_sp_msg_send_direct_req(&req, &resp, SP_RESULT_OK);
	LONGS_EQUAL(TS_RPC_CALL_ACCEPTED,
		    rpc_caller_invoke(rpc_caller_instance, handle, opcode,
				      &opstatus, &resp_buf, &resp_len));
	POINTERS_EQUAL(shared_buffer, resp_buf);
	UNSIGNED_LONGS_EQUAL(sizeof(shared_buffer), resp_len);
}

TEST(ffarpc_caller, invoke_without_response)
{
	rpc_call_handle handle = &caller;
	uint16_t opcode = 0xfedc;
	rpc_opstatus_t opstatus = 0;
	uint8_t *resp_buf = (uint8_t *)0x123;
	size_t resp_len = 1;

	caller.is_call_transaction_in_progess = true;
	caller.shared_mem_handle = 0x56789abcdef01234ULL;
	caller.dest_partition_id = 0x1234;
	caller.req_len = 0x3456789a;
	caller.rpc_caller.encoding = 0xabcdef12;

	req.source_id = own_id;
	req.destination_id = caller.dest_partition_id;
	req.args.args32[0] = opcode;
	req.args.args32[1] = caller.req_len;
	req.args.args32[2] = 0;
	req.args.args32[3] = caller.rpc_caller.encoding;

	resp.source_id = caller.dest_partition_id;
	resp.destination_id = own_id;
	resp.args.args32[0] = opcode;
	resp.args.args32[1] = 0;
	resp.args.args32[2] = TS_RPC_CALL_ACCEPTED;
	resp.args.args32[3] = 0;

	expect_sp_msg_send_direct_req(&req, &resp, SP_RESULT_OK);
	LONGS_EQUAL(TS_RPC_CALL_ACCEPTED,
		    rpc_caller_invoke(rpc_caller_instance, handle, opcode,
				      &opstatus, &resp_buf, &resp_len));
	POINTERS_EQUAL(NULL, resp_buf);
	UNSIGNED_LONGS_EQUAL(0, resp_len);
}

TEST(ffarpc_caller, end_null_context)
{
	rpc_call_handle handle = &caller;

	caller.rpc_caller.context = NULL;
	rpc_caller_end(rpc_caller_instance, handle);
}

TEST(ffarpc_caller, end_null_handle)
{
	rpc_caller_end(rpc_caller_instance, NULL);
}

TEST(ffarpc_caller, end)
{
	rpc_call_handle handle = &caller;

	caller.is_call_transaction_in_progess = true;
	rpc_caller_end(rpc_caller_instance, handle);
	CHECK_FALSE(caller.is_call_transaction_in_progess);
}
