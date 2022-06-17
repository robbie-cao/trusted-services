/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include "../ffarpc_call_ep.h"
#include "mock_sp_memory_management.h"
#include "mock_rpc_interface.h"
#include "call_param_buf_comparator.h"
#include <string.h>

TEST_GROUP(ffarpc_call_ep) {
	TEST_SETUP()
	{
		rpc_iface.receive = mock_rpc_interface_receive;
		ffa_call_ep_init(&ep, &rpc_iface, dst);

		mock().installComparator("call_req", call_req_buf_comparator);

		memset(&req, 0x00, sizeof(req));
		memset(&resp, 0x00, sizeof(resp));
		memset(&mem_desc, 0x00, sizeof(mem_desc));
		memset(&mem_acc_desc, 0x00, sizeof(mem_acc_desc));
		memset(&mem_region, 0x00, sizeof(mem_region));
		out_region_count = 0;
	}

	TEST_TEARDOWN()
	{
		mock().checkExpectations();
		mock().removeAllComparatorsAndCopiers();
		mock().clear();
	}

	void do_mem_share(uint16_t source_id, uint16_t dest_id, uint64_t handle,
			  uint32_t size)
	{
		req.source_id = source_id;
		req.destination_id = dest_id;
		req.args.args32[0] = (0x1000 << 16) | 0x0; // FFA_CALL_OPCODE_SHARE_BUF
		req.args.args32[1] = handle & 0xffffffff;
		req.args.args32[2] = (handle >> 32) & 0xffffffff;
		req.args.args32[3] = size;

		memset(&resp, 0x00, sizeof(resp));

		ffa_call_ep_receive(&ep, &req, &resp);
	}

	void check_mem_share_response(uint16_t source_id, uint16_t dest_id,
				      rpc_status_t rpc_status)
	{
		check_response(source_id, dest_id, 0x1000, 0x0, 0, rpc_status, 0);
	}

	void do_mem_unshare(uint16_t source_id, uint16_t dest_id)
	{
		req.source_id = source_id;
		req.destination_id = dest_id;
		req.args.args32[0] = (0x1000 << 16) | 0x1; // FFA_CALL_OPCODE_UNSHARE_BUF
		req.args.args32[1] = 0;
		req.args.args32[2] = 0;
		req.args.args32[3] = 0;

		memset(&resp, 0x00, sizeof(resp));

		ffa_call_ep_receive(&ep, &req, &resp);
	}

	void check_mem_unshare_response(uint16_t source_id, uint16_t dest_id,
					rpc_status_t rpc_status)
	{
		check_response(source_id, dest_id, 0x1000, 0x1, 0, rpc_status, 0);
	}

	void do_request(uint16_t source_id, uint16_t dest_id, uint16_t iface_id,
			uint16_t opcode, uint32_t req_len, uint32_t encoding)
	{
		req.source_id = source_id;
		req.destination_id = dest_id;
		req.args.args32[0] = (iface_id << 16) | opcode;
		req.args.args32[1] = req_len;
		req.args.args32[2] = 0; // Caller id
		req.args.args32[3] = encoding;

		memset(&resp, 0x00, sizeof(resp));

		ffa_call_ep_receive(&ep, &req, &resp);
	}

	void check_response(uint16_t source_id, uint16_t dest_id, uint16_t iface_id,
			    uint16_t opcode, uint32_t data_len,
			    rpc_status_t rpc_status, rpc_opstatus_t opstatus)
	{
		UNSIGNED_LONGLONGS_EQUAL(source_id, resp.source_id);
		UNSIGNED_LONGLONGS_EQUAL(dest_id, resp.destination_id);
		UNSIGNED_LONGS_EQUAL(iface_id, (resp.args.args32[0] >> 16) & 0xffff);
		UNSIGNED_LONGS_EQUAL(opcode, resp.args.args32[0] & 0xffff);
		UNSIGNED_LONGS_EQUAL(data_len, (uint32_t)resp.args.args32[1]);
		LONGS_EQUAL(rpc_status, (rpc_status_t)resp.args.args32[2]);
		LONGS_EQUAL(opstatus, (rpc_opstatus_t)resp.args.args32[3]);
	}

	void expect_retrieve(uint16_t source_id, uint16_t dest_id, uint64_t handle,
			     void *address, size_t page_count, sp_result result)
	{

		mem_desc.sender_id = source_id;
		mem_desc.memory_type = sp_memory_type_not_specified;
		mem_desc.flags.transaction_type = sp_memory_transaction_type_share;
		mem_acc_desc.receiver_id = dest_id;
		mem_acc_desc.instruction_access = sp_instruction_access_not_specified;
		mem_acc_desc.data_access = sp_data_access_read_write;
		mem_region.address = address;
		mem_region.page_count = page_count;

		out_region_count = 1;

		expect_sp_memory_retrieve(&mem_desc, &mem_acc_desc, &mem_acc_desc,
					  NULL, &mem_region, 0, &out_region_count, handle, result);
	}

	struct rpc_interface rpc_iface;
	struct ffa_call_ep ep;
	struct sp_msg req;
	struct sp_msg resp;
	struct sp_memory_descriptor mem_desc;
	struct sp_memory_access_descriptor mem_acc_desc;
	struct sp_memory_region mem_region;
	uint32_t out_region_count;

	static const uint16_t src = 0x1234;
	static const uint16_t dst = 0x5678;
	static const uint64_t handle = 0xabcdef0123456789ULL;

	call_param_buf_comparator call_req_buf_comparator;
};

TEST(ffarpc_call_ep, mem_share)
{
	expect_retrieve(src, dst, handle, (void *)0x123456789, 1, SP_RESULT_OK);
	do_mem_share(src, dst, handle, 4096);
	check_mem_share_response(dst, src, TS_RPC_CALL_ACCEPTED);
}

TEST(ffarpc_call_ep, mem_share_fill_all)
{
	for (uint16_t i = 0; i < NUM_MAX_SESS; i++) {
		expect_retrieve(src + i, dst, handle, (void *)0x123456789, 1, SP_RESULT_OK);
		do_mem_share(src + i, dst, handle, 4096);
		check_mem_share_response(dst, src + i, TS_RPC_CALL_ACCEPTED);
	}

	do_mem_share(src + NUM_MAX_SESS, dst, handle, 4096);
	check_mem_share_response(dst, src + NUM_MAX_SESS, TS_RPC_ERROR_INTERNAL);
}

TEST(ffarpc_call_ep, mem_share_retrieve_fail)
{
	expect_retrieve(src, dst, handle, (void *)0x123456789, 1, SP_RESULT_INVALID_PARAMETERS);
	do_mem_share(src, dst, handle, 4096);
	check_mem_share_response(dst, src, TS_RPC_ERROR_INTERNAL);
}

TEST(ffarpc_call_ep, mem_share_smaller_page_count)
{
	const uint16_t endpoints[1] = { dst };
	const struct sp_memory_transaction_flags flags = {
		.zero_memory = false,
		.operation_time_slicing = false,
	};
	expect_retrieve(src, dst, handle, (void *)0x123456789, 1, SP_RESULT_OK);
	expect_sp_memory_relinquish(handle, endpoints, 1, &flags, SP_RESULT_OK);
	do_mem_share(src, dst, handle, 4096 * 2);
	check_mem_share_response(dst, src, TS_RPC_ERROR_INVALID_PARAMETER);
}

TEST(ffarpc_call_ep, mem_share_smaller_page_count_relinquish_fail)
{
	const uint16_t endpoints[1] = { dst };
	const struct sp_memory_transaction_flags flags = {
		.zero_memory = false,
		.operation_time_slicing = false,
	};
	expect_retrieve(src, dst, handle, (void *)0x123456789, 1, SP_RESULT_OK);
	expect_sp_memory_relinquish(handle, endpoints, 1, &flags, SP_RESULT_INVALID_PARAMETERS);
	do_mem_share(src, dst, handle, 4096 * 2);
	check_mem_share_response(dst, src, TS_RPC_ERROR_INVALID_PARAMETER);
}

TEST(ffarpc_call_ep, mem_share_null_ep)
{
	req.source_id = src;
	req.destination_id = dst;
	req.args.args32[0] = (0x1000 << 16) | 0;

	ffa_call_ep_receive(NULL, &req, &resp);
}

TEST(ffarpc_call_ep, mem_unshare)
{
	do_mem_unshare(src, dst);
	check_mem_unshare_response(dst, src, TS_RPC_ERROR_INTERNAL);
}

TEST(ffarpc_call_ep, mem_share_unshare)
{
	const uint16_t endpoints[1] = { dst };
	const struct sp_memory_transaction_flags flags = {
		.zero_memory = false,
		.operation_time_slicing = false,
	};

	expect_retrieve(src, dst, handle, (void *)0x123456789, 1, SP_RESULT_OK);
	do_mem_share(src, dst, handle, 4096);
	check_mem_share_response(dst, src, TS_RPC_CALL_ACCEPTED);

	expect_sp_memory_relinquish(handle, endpoints, 1, &flags, SP_RESULT_OK);
	do_mem_unshare(src, dst);
	check_mem_unshare_response(dst, src, TS_RPC_CALL_ACCEPTED);
}

TEST(ffarpc_call_ep, mem_share_unshare_relinquish_fail)
{
	const uint16_t endpoints[1] = { dst };
	const struct sp_memory_transaction_flags flags = {
		.zero_memory = false,
		.operation_time_slicing = false,
	};

	expect_retrieve(src, dst, handle, (void *)0x123456789, 1, SP_RESULT_OK);
	do_mem_share(src, dst, handle, 4096);
	check_mem_share_response(dst, src, TS_RPC_CALL_ACCEPTED);

	expect_sp_memory_relinquish(handle, endpoints, 1, &flags, SP_RESULT_INVALID_PARAMETERS);
	do_mem_unshare(src, dst);
	check_mem_unshare_response(dst, src, TS_RPC_ERROR_INTERNAL);
}

TEST(ffarpc_call_ep, mem_share_unshare_relinquish_fail_then_success)
{
	const uint16_t endpoints[1] = { dst };
	const struct sp_memory_transaction_flags flags = {
		.zero_memory = false,
		.operation_time_slicing = false,
	};

	expect_retrieve(src, dst, handle, (void *)0x123456789, 1, SP_RESULT_OK);
	do_mem_share(src, dst, handle, 4096);
	check_mem_share_response(dst, src, TS_RPC_CALL_ACCEPTED);

	expect_sp_memory_relinquish(handle, endpoints, 1, &flags, SP_RESULT_INVALID_PARAMETERS);
	do_mem_unshare(src, dst);
	check_mem_unshare_response(dst, src, TS_RPC_ERROR_INTERNAL);

	expect_sp_memory_relinquish(handle, endpoints, 1, &flags, SP_RESULT_OK);
	do_mem_unshare(src, dst);
	check_mem_unshare_response(dst, src, TS_RPC_CALL_ACCEPTED);
}

TEST(ffarpc_call_ep, mem_unshare_null_ep)
{
	req.source_id = src;
	req.destination_id = dst;
	req.args.args32[0] = (0x1000 << 16) | 1;

	ffa_call_ep_receive(NULL, &req, &resp);
}

TEST(ffarpc_call_ep, invalid_mgmt_call)
{
	do_request(src, dst, 0x1000, 0xffff, 0, 0);
	check_response(dst, src, 0x1000, 0xffff, 0, TS_RPC_ERROR_INVALID_OPCODE, 0);
}

TEST(ffarpc_call_ep, request_without_data)
{
	const struct call_req rpc_req = {
		.caller_id = src,
		.interface_id = 0xfedc,
		.opcode = 0xbaab,
		.encoding = 0x12345678,
		.opstatus = 0,
		.req_buf = {
			.size = 0,
			.data_len = 0,
			.data = NULL,
		},
		.resp_buf = {
			.size = 0,
			.data_len = 0,
			.data = NULL,
		}
	};

	expect_mock_rpc_interface_receive(&rpc_iface, &rpc_req, TS_RPC_CALL_ACCEPTED);
	do_request(src, dst, rpc_req.interface_id, rpc_req.opcode, 0, rpc_req.encoding);
	check_response(dst, src, rpc_req.interface_id, rpc_req.opcode, 0, TS_RPC_CALL_ACCEPTED, 0);
}

TEST(ffarpc_call_ep, request_without_data_non_zero_length)
{
	const struct call_req rpc_req = {
		.caller_id = src,
		.interface_id = 0xfedc,
		.opcode = 0xbaab,
		.encoding = 0x12345678,
		.opstatus = 0,
		.req_buf = {
			.size = 0,
			.data_len = 0,
			.data = NULL,
		},
		.resp_buf = {
			.size = 0,
			.data_len = 0,
			.data = NULL,
		}
	};

	do_request(src, dst, rpc_req.interface_id, rpc_req.opcode, 1, rpc_req.encoding);
	check_response(dst, src, rpc_req.interface_id, rpc_req.opcode, 0, TS_RPC_ERROR_INVALID_PARAMETER, 0);
}

TEST(ffarpc_call_ep, request_with_data)
{
	const struct call_req rpc_req = {
		.caller_id = src,
		.interface_id = 0xfedc,
		.opcode = 0xbaab,
		.encoding = 0x12345678,
		.opstatus = 0,
		.req_buf = {
			.size = 4096,
			.data_len = 100,
			.data = (void *)0x123456789,
		},
		.resp_buf = {
			.size = 4096,
			.data_len = 0,
			.data = (void *)0x123456789,
		}
	};

	expect_retrieve(src, dst, handle, rpc_req.req_buf.data, 1, SP_RESULT_OK);
	do_mem_share(src, dst, handle, 4096);
	check_mem_share_response(dst, src, TS_RPC_CALL_ACCEPTED);

	expect_mock_rpc_interface_receive(&rpc_iface, &rpc_req, TS_RPC_CALL_ACCEPTED);
	do_request(src, dst, rpc_req.interface_id, rpc_req.opcode, 100, rpc_req.encoding);
	check_response(dst, src, rpc_req.interface_id, rpc_req.opcode, 0, TS_RPC_CALL_ACCEPTED, 0);
}

TEST(ffarpc_call_ep, request_with_data_buffer_overflow)
{
	const struct call_req rpc_req = {
		.caller_id = src,
		.interface_id = 0xfedc,
		.opcode = 0xbaab,
		.encoding = 0x12345678,
		.opstatus = 0,
		.req_buf = {
			.size = 4096,
			.data_len = 100,
			.data = (void *)0x123456789,
		},
		.resp_buf = {
			.size = 4096,
			.data_len = 0,
			.data = (void *)0x123456789,
		}
	};

	expect_retrieve(src, dst, handle, rpc_req.req_buf.data, 1, SP_RESULT_OK);
	do_mem_share(src, dst, handle, 4096);
	check_mem_share_response(dst, src, TS_RPC_CALL_ACCEPTED);

	do_request(src, dst, rpc_req.interface_id, rpc_req.opcode, 4097, rpc_req.encoding);
	check_response(dst, src, rpc_req.interface_id, rpc_req.opcode, 0, TS_RPC_ERROR_INVALID_PARAMETER, 0);
}

TEST(ffarpc_call_ep, request_with_data_after_unshare)
{
	const uint16_t endpoints[1] = { dst };
	const struct sp_memory_transaction_flags flags = {
		.zero_memory = false,
		.operation_time_slicing = false,
	};
	const struct call_req rpc_req = {
		.caller_id = src,
		.interface_id = 0xfedc,
		.opcode = 0xbaab,
		.encoding = 0x12345678,
		.opstatus = 0,
		.req_buf = {
			.size = 4096,
			.data_len = 100,
			.data = (void *)0x123456789,
		},
		.resp_buf = {
			.size = 4096,
			.data_len = 0,
			.data = (void *)0x123456789,
		}
	};

	expect_retrieve(src, dst, handle, rpc_req.req_buf.data, 1, SP_RESULT_OK);
	do_mem_share(src, dst, handle, 4096);
	check_mem_share_response(dst, src, TS_RPC_CALL_ACCEPTED);

	expect_sp_memory_relinquish(handle, endpoints, 1, &flags, SP_RESULT_OK);
	do_mem_unshare(src, dst);
	check_mem_unshare_response(dst, src, TS_RPC_CALL_ACCEPTED);

	do_request(src, dst, rpc_req.interface_id, rpc_req.opcode, 100, rpc_req.encoding);
	check_response(dst, src, rpc_req.interface_id, rpc_req.opcode, 0, TS_RPC_ERROR_INVALID_PARAMETER, 0);
}

TEST(ffarpc_call_ep, request_deny_64_bit)
{
	req.source_id = src;
	req.destination_id = dst;
	req.is_64bit_message = true;

	memset(&resp, 0x5a, sizeof(resp));

	ffa_call_ep_receive(&ep, &req, &resp);

	UNSIGNED_LONGS_EQUAL(dst, resp.source_id);
	UNSIGNED_LONGS_EQUAL(src, resp.destination_id);
	UNSIGNED_LONGS_EQUAL(0, resp.args.args64[0]);
	UNSIGNED_LONGLONGS_EQUAL(0, resp.args.args64[1]);
	LONGLONGS_EQUAL(TS_RPC_ERROR_INVALID_PARAMETER, resp.args.args64[2]);
	UNSIGNED_LONGLONGS_EQUAL(0, resp.args.args64[3]);
}
