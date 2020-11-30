// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2020-2021, Arm Limited. All rights reserved.
 */

#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <stdint.h>
#include <string.h>
#include "ffa_api.h"
#include "mock_assert.h"
#include "mock_ffa_internal_api.h"

void expect_ffa_interrupt_handler(uint32_t id)
{
	mock().expectOneCall("ffa_interrupt_handler")
		.withUnsignedIntParameter("id", id);
}

void ffa_interrupt_handler(uint32_t id)
{
	mock().actualCall("ffa_interrupt_handler")
		.withUnsignedIntParameter("id", id);
}

TEST_GROUP(ffa_api)
{
	TEST_SETUP()
	{
		memset(&svc_result, 0x00, sizeof(svc_result));
		memset(&msg, 0x00, sizeof(msg));
	}

	TEST_TEARDOWN()
	{
		mock().checkExpectations();
		mock().clear();
	}

	void setup_error_response(int32_t error_code)
	{
		svc_result.a0 = 0x84000060;
		svc_result.a2 = (uint32_t)error_code;
	}

	void msg_equal(uint32_t func_id, uint16_t source_id, uint16_t dest_id,
			uint32_t arg0, uint32_t arg1, uint32_t arg2,
			uint32_t arg3, uint32_t arg4)
	{
		UNSIGNED_LONGS_EQUAL(func_id, msg.function_id);
		UNSIGNED_LONGS_EQUAL(source_id, msg.source_id);
		UNSIGNED_LONGS_EQUAL(dest_id, msg.destination_id);
		UNSIGNED_LONGS_EQUAL(arg0, msg.args[0]);
		UNSIGNED_LONGS_EQUAL(arg1, msg.args[1]);
		UNSIGNED_LONGS_EQUAL(arg2, msg.args[2]);
		UNSIGNED_LONGS_EQUAL(arg3, msg.args[3]);
		UNSIGNED_LONGS_EQUAL(arg4, msg.args[4]);
	}

	struct ffa_params svc_result;
	struct ffa_direct_msg msg;
};

TEST(ffa_api, ffa_version)
{
	uint32_t version = 0;
	const uint32_t test_version = 0x78901234;

	svc_result.a0 = test_version;
	expect_ffa_svc(0x84000063, 0x10000U, 0, 0, 0, 0, 0, 0, &svc_result);

	ffa_result result = ffa_version(&version);
	LONGS_EQUAL(FFA_OK, result);
	UNSIGNED_LONGS_EQUAL(test_version, version);
}

TEST(ffa_api, ffa_version_error)
{
	uint32_t version = 0;

	svc_result.a0 = 0xffffffff;
	expect_ffa_svc(0x84000063, 0x10000U, 0, 0, 0, 0, 0, 0, &svc_result);

	ffa_result result = ffa_version(&version);
	LONGS_EQUAL(-1, result);
	UNSIGNED_LONGS_EQUAL(0, version);
}

TEST(ffa_api, ffa_features)
{
	const uint32_t func_id = 0x11223344;
	const uint32_t features0 = 0x01234567;
	const uint32_t features1 = 0x89abcdef;
	struct ffa_interface_properties properties = { 0xffffffff, 0xffffffff };

	svc_result.a0 = 0x84000061;
	svc_result.a2 = features0;
	svc_result.a3 = features1;
	expect_ffa_svc(0x84000064, func_id, 0, 0, 0, 0, 0, 0, &svc_result);

	ffa_result result = ffa_features(func_id, &properties);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGS_EQUAL(features0, properties.interface_properties[0]);
	UNSIGNED_LONGS_EQUAL(features1, properties.interface_properties[1]);
}

TEST(ffa_api, ffa_features_error)
{
	const uint32_t func_id = 0x11223344;
	struct ffa_interface_properties properties = { 0xffffffff, 0xffffffff };

	setup_error_response(-1);
	expect_ffa_svc(0x84000064, func_id, 0, 0, 0, 0, 0, 0, &svc_result);

	ffa_result result = ffa_features(func_id, &properties);
	LONGS_EQUAL(-1, result);
	UNSIGNED_LONGS_EQUAL(0, properties.interface_properties[0]);
	UNSIGNED_LONGS_EQUAL(0, properties.interface_properties[1]);
}

TEST(ffa_api, ffa_features_unknown_response)
{
	const uint32_t func_id = 0x11223344;
	struct ffa_interface_properties properties = { 0, 0 };
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000064, func_id, 0, 0, 0, 0, 0, 0, &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_features(func_id, &properties);
	}
}

TEST(ffa_api, ffa_rx_release)
{
	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x84000065, 0, 0, 0, 0, 0, 0, 0, &svc_result);

	ffa_result result = ffa_rx_release();
	LONGS_EQUAL(FFA_OK, result);
}

TEST(ffa_api, ffa_rx_release_error)
{
	setup_error_response(-1);
	expect_ffa_svc(0x84000065, 0, 0, 0, 0, 0, 0, 0, &svc_result);

	ffa_result result = ffa_rx_release();
	LONGS_EQUAL(-1, result);
}

TEST(ffa_api, ffa_rx_release_unknown_response)
{
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000065, 0, 0, 0, 0, 0, 0, 0, &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_rx_release();
	}
}

TEST(ffa_api, ffa_rxtx_map)
{
	const uint64_t tx_buffer = 0xfedcba9876543210ULL;
	const uint64_t rx_buffer = 0x0123456789abcdefULL;
	const uint64_t page_count = (1 << 6) - 1;

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x84000066, tx_buffer, rx_buffer, page_count, 0, 0, 0, 0,
		       &svc_result);

	ffa_result result = ffa_rxtx_map((const void *)tx_buffer,
					 (const void *)rx_buffer, page_count);
	LONGS_EQUAL(0, result);
}

TEST(ffa_api, ffa_rxtx_map_error)
{
	const uint64_t tx_buffer = 0xfedcba9876543210ULL;
	const uint64_t rx_buffer = 0x0123456789abcdefULL;
	const uint64_t page_count = (1 << 6) - 1;

	setup_error_response(-1);
	expect_ffa_svc(0x84000066, tx_buffer, rx_buffer, page_count, 0, 0, 0, 0,
		       &svc_result);

	ffa_result result = ffa_rxtx_map((const void *)tx_buffer,
					 (const void *)rx_buffer, page_count);
	LONGS_EQUAL(-1, result);
}

TEST(ffa_api, ffa_rxtx_map_unknown_response)
{
	const uint64_t tx_buffer = 0xfedcba9876543210ULL;
	const uint64_t rx_buffer = 0x0123456789abcdefULL;
	const uint64_t page_count = (1 << 6) - 1;
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000066, tx_buffer, rx_buffer, page_count, 0, 0, 0, 0,
		       &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_rxtx_map((const void *)tx_buffer, (const void *)rx_buffer,
			     page_count);
	}
}

TEST(ffa_api, ffa_rxtx_map_too_many_pages)
{
	const uint64_t tx_buffer = 0xfedcba9876543210ULL;
	const uint64_t rx_buffer = 0x0123456789abcdefULL;
	const uint64_t page_count = (1 << 6);
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_rxtx_map((const void *)tx_buffer, (const void *)rx_buffer,
			     page_count);
	}
}

TEST(ffa_api, ffa_rxtx_unmap)
{
	const uint16_t id = 0xffff;

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x84000067, ((uint32_t)id) << 16, 0, 0, 0, 0, 0, 0,
		       &svc_result);

	ffa_result result = ffa_rxtx_unmap(id);
	LONGS_EQUAL(FFA_OK, result);
}

TEST(ffa_api, ffa_rxtx_unmap_error)
{
	const uint16_t id = 0xffff;

	setup_error_response(-1);
	expect_ffa_svc(0x84000067, ((uint32_t)id) << 16, 0, 0, 0, 0, 0, 0,
		       &svc_result);

	ffa_result result = ffa_rxtx_unmap(id);
	LONGS_EQUAL(-1, result);
}

TEST(ffa_api, ffa_rxtx_unmap_unknown_response)
{
	const uint16_t id = 0xffff;
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000067, ((uint32_t)id) << 16, 0, 0, 0, 0, 0, 0,
		       &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_rxtx_unmap(id);
	}
}

TEST(ffa_api, ffa_partition_info_get)
{
	const struct ffa_uuid uuid = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
				       0xcd, 0xef, 0x12, 0x34, 0x56, 0x78,
				       0x9a, 0xbc, 0xde, 0xf0 };
	uint32_t count = 0;
	const uint32_t count_result = 0xaabbccdd;

	svc_result.a0 = 0x84000061;
	svc_result.a2 = count_result;
	expect_ffa_svc(0x84000068, 0x67452301, 0xefcdab89, 0x78563412,
		       0xf0debc9a, 0, 0, 0, &svc_result);

	ffa_result result = ffa_partition_info_get(&uuid, &count);
	LONGS_EQUAL(FFA_OK, result);
	UNSIGNED_LONGS_EQUAL(count_result, count);
}

TEST(ffa_api, ffa_partition_info_get_error)
{
	const struct ffa_uuid uuid = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
				       0xcd, 0xef, 0x12, 0x34, 0x56, 0x78,
				       0x9a, 0xbc, 0xde, 0xf0 };
	uint32_t count = 0x1234;

	setup_error_response(-1);
	expect_ffa_svc(0x84000068, 0x67452301, 0xefcdab89, 0x78563412,
		       0xf0debc9a, 0, 0, 0, &svc_result);

	ffa_result result = ffa_partition_info_get(&uuid, &count);
	LONGS_EQUAL(-1, result);
	UNSIGNED_LONGS_EQUAL(0, count);
}

TEST(ffa_api, ffa_partition_info_get_unknown_response)
{
	const struct ffa_uuid uuid = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
				       0xcd, 0xef, 0x12, 0x34, 0x56, 0x78,
				       0x9a, 0xbc, 0xde, 0xf0 };
	uint32_t count = 0;
	const uint32_t count_result = 0xaabbccdd;
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	svc_result.a2 = count_result;
	expect_ffa_svc(0x84000068, 0x67452301, 0xefcdab89, 0x78563412,
		       0xf0debc9a, 0, 0, 0, &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_partition_info_get(&uuid, &count);
	}
}

TEST(ffa_api, ffa_id_get)
{
	uint16_t id = 0;
	const uint16_t id_result = 0xfedc;

	svc_result.a0 = 0x84000061;
	svc_result.a2 = id_result;
	expect_ffa_svc(0x84000069, 0, 0, 0, 0, 0, 0, 0, &svc_result);

	ffa_result result = ffa_id_get(&id);
	LONGS_EQUAL(FFA_OK, result);
	UNSIGNED_LONGS_EQUAL(id_result, id);
}

TEST(ffa_api, ffa_id_get_error)
{
	uint16_t id = 0;

	setup_error_response(-1);
	expect_ffa_svc(0x84000069, 0, 0, 0, 0, 0, 0, 0, &svc_result);

	ffa_result result = ffa_id_get(&id);
	LONGS_EQUAL(-1, result);
	UNSIGNED_LONGS_EQUAL(0xffff, id);
}

TEST(ffa_api, ffa_id_get_unknown_response)
{
	uint16_t id = 0xffff;
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000069, 0, 0, 0, 0, 0, 0, 0, &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_id_get(&id);
	}
}

TEST(ffa_api, ffa_msg_wait_success)
{
	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &svc_result);
	ffa_result result = ffa_msg_wait(&msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x84000061, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_wait_error)
{
	setup_error_response(-1);
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &svc_result);
	ffa_result result = ffa_msg_wait(&msg);
	LONGS_EQUAL(-1, result);
	msg_equal(0, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_wait_direct_req)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;

	svc_result.a0 = 0x8400006F;
	svc_result.a1 = ((uint32_t)source_id) << 16 | dest_id;
	svc_result.a3 = arg0;
	svc_result.a4 = arg1;
	svc_result.a5 = arg2;
	svc_result.a6 = arg3;
	svc_result.a7 = arg4;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &svc_result);
	ffa_result result = ffa_msg_wait(&msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x8400006F, source_id, dest_id, arg0, arg1, arg2, arg3,
		   arg4);
}

TEST(ffa_api, ffa_msg_wait_one_interrupt_success)
{
	const uint32_t interrupt_id = 0x12345678;
	struct ffa_params interrupt_params;

	interrupt_params.a0 = 0x84000062;
	interrupt_params.a2 = interrupt_id;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &interrupt_params);
	expect_ffa_interrupt_handler(interrupt_id);

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &svc_result);
	ffa_result result = ffa_msg_wait(&msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x84000061, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_wait_two_interrupt_success)
{
	const uint32_t interrupt_id0 = 0x12345678;
	const uint32_t interrupt_id1 = 0x81234567;
	struct ffa_params interrupt_params0;
	struct ffa_params interrupt_params1;

	interrupt_params0.a0 = 0x84000062;
	interrupt_params0.a2 = interrupt_id0;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &interrupt_params0);
	expect_ffa_interrupt_handler(interrupt_id0);

	interrupt_params1.a0 = 0x84000062;
	interrupt_params1.a2 = interrupt_id1;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &interrupt_params1);
	expect_ffa_interrupt_handler(interrupt_id1);

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &svc_result);
	ffa_result result = ffa_msg_wait(&msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x84000061, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_wait_unknown_response)
{
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &svc_result);
	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_msg_wait(&msg);
	}
}

TEST(ffa_api, ffa_msg_send_direct_req_success)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x8400006F, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &svc_result);
	ffa_result result = ffa_msg_send_direct_req(
		source_id, dest_id, arg0, arg1, arg2, arg3, arg4, &msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x84000061, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_send_direct_req_error)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;

	setup_error_response(-1);
	expect_ffa_svc(0x8400006F, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &svc_result);
	ffa_result result = ffa_msg_send_direct_req(
		source_id, dest_id, arg0, arg1, arg2, arg3, arg4, &msg);
	LONGS_EQUAL(-1, result);
	msg_equal(0, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_send_direct_req_direct_resp)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;
	const uint16_t resp_source_id = 0x1221;
	const uint16_t resp_dest_id = 0x3443;
	const uint32_t resp_arg0 = 0x90123456ULL;
	const uint32_t resp_arg1 = 0xa1234567ULL;
	const uint32_t resp_arg2 = 0xb2345678ULL;
	const uint32_t resp_arg3 = 0xc3456789ULL;
	const uint32_t resp_arg4 = 0xd456789aULL;

	svc_result.a0 = 0x84000070;
	svc_result.a1 = ((uint32_t)resp_source_id) << 16 | resp_dest_id;
	svc_result.a3 = resp_arg0;
	svc_result.a4 = resp_arg1;
	svc_result.a5 = resp_arg2;
	svc_result.a6 = resp_arg3;
	svc_result.a7 = resp_arg4;
	expect_ffa_svc(0x8400006F, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &svc_result);
	ffa_result result = ffa_msg_send_direct_req(
		source_id, dest_id, arg0, arg1, arg2, arg3, arg4, &msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x84000070, resp_source_id, resp_dest_id, resp_arg0,
		   resp_arg1, resp_arg2, resp_arg3, resp_arg4);
}

TEST(ffa_api, ffa_msg_send_direct_req_one_interrupt_success)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;
	const uint32_t interrupt_id = 0x12345678;
	struct ffa_params interrupt_params;

	interrupt_params.a0 = 0x84000062;
	interrupt_params.a2 = interrupt_id;
	expect_ffa_svc(0x8400006F, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &interrupt_params);
	expect_ffa_interrupt_handler(interrupt_id);

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &svc_result);
	ffa_result result = ffa_msg_send_direct_req(
		source_id, dest_id, arg0, arg1, arg2, arg3, arg4, &msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x84000061, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_send_direct_req_two_interrupt_success)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;
	const uint32_t interrupt_id = 0x12345678;
	const uint32_t interrupt_id0 = 0x12345678;
	const uint32_t interrupt_id1 = 0x81234567;
	struct ffa_params interrupt_params0;
	struct ffa_params interrupt_params1;

	interrupt_params0.a0 = 0x84000062;
	interrupt_params0.a2 = interrupt_id0;
	expect_ffa_svc(0x8400006F, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &interrupt_params0);
	expect_ffa_interrupt_handler(interrupt_id0);

	interrupt_params1.a0 = 0x84000062;
	interrupt_params1.a2 = interrupt_id1;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &interrupt_params1);
	expect_ffa_interrupt_handler(interrupt_id1);

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &svc_result);
	ffa_result result = ffa_msg_send_direct_req(
		source_id, dest_id, arg0, arg1, arg2, arg3, arg4, &msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x84000061, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_send_direct_req_unknown_response)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x8400006F, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &svc_result);
	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_msg_send_direct_req(source_id, dest_id, arg0, arg1, arg2,
					arg3, arg4, &msg);
	}
}

TEST(ffa_api, ffa_msg_send_direct_resp_success)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x84000070, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &svc_result);
	ffa_result result = ffa_msg_send_direct_resp(
		source_id, dest_id, arg0, arg1, arg2, arg3, arg4, &msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x84000061, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_send_direct_resp_error)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;

	setup_error_response(-1);
	expect_ffa_svc(0x84000070, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &svc_result);
	ffa_result result = ffa_msg_send_direct_resp(
		source_id, dest_id, arg0, arg1, arg2, arg3, arg4, &msg);
	LONGS_EQUAL(-1, result);
	msg_equal(0, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_send_direct_resp_then_get_direct_req_as_response)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;
	const uint16_t resp_source_id = 0x1221;
	const uint16_t resp_dest_id = 0x3443;
	const uint32_t resp_arg0 = 0x90123456ULL;
	const uint32_t resp_arg1 = 0xa1234567ULL;
	const uint32_t resp_arg2 = 0xb2345678ULL;
	const uint32_t resp_arg3 = 0xc3456789ULL;
	const uint32_t resp_arg4 = 0xd456789aULL;

	svc_result.a0 = 0x8400006F;
	svc_result.a1 = ((uint32_t)resp_source_id) << 16 | resp_dest_id;
	svc_result.a3 = resp_arg0;
	svc_result.a4 = resp_arg1;
	svc_result.a5 = resp_arg2;
	svc_result.a6 = resp_arg3;
	svc_result.a7 = resp_arg4;
	expect_ffa_svc(0x84000070, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &svc_result);
	ffa_result result = ffa_msg_send_direct_resp(
		source_id, dest_id, arg0, arg1, arg2, arg3, arg4, &msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x8400006F, resp_source_id, resp_dest_id, resp_arg0,
		   resp_arg1, resp_arg2, resp_arg3, resp_arg4);
}

TEST(ffa_api, ffa_msg_send_direct_resp_one_interrupt_success)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;
	const uint32_t interrupt_id = 0x12345678;
	struct ffa_params interrupt_params;

	interrupt_params.a0 = 0x84000062;
	interrupt_params.a2 = interrupt_id;
	expect_ffa_svc(0x84000070, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &interrupt_params);
	expect_ffa_interrupt_handler(interrupt_id);

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &svc_result);
	ffa_result result = ffa_msg_send_direct_resp(
		source_id, dest_id, arg0, arg1, arg2, arg3, arg4, &msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x84000061, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_send_direct_resp_two_interrupt_success)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;
	const uint32_t interrupt_id = 0x12345678;
	const uint32_t interrupt_id0 = 0x12345678;
	const uint32_t interrupt_id1 = 0x81234567;
	struct ffa_params interrupt_params0;
	struct ffa_params interrupt_params1;

	interrupt_params0.a0 = 0x84000062;
	interrupt_params0.a2 = interrupt_id0;
	expect_ffa_svc(0x84000070, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &interrupt_params0);
	expect_ffa_interrupt_handler(interrupt_id0);

	interrupt_params1.a0 = 0x84000062;
	interrupt_params1.a2 = interrupt_id1;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &interrupt_params1);
	expect_ffa_interrupt_handler(interrupt_id1);

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x8400006B, 0, 0, 0, 0, 0, 0, 0, &svc_result);
	ffa_result result = ffa_msg_send_direct_resp(
		source_id, dest_id, arg0, arg1, arg2, arg3, arg4, &msg);
	LONGS_EQUAL(0, result);
	msg_equal(0x84000061, 0, 0, 0, 0, 0, 0, 0);
}

TEST(ffa_api, ffa_msg_send_direct_resp_unknown_response)
{
	const uint16_t source_id = 0x1122;
	const uint16_t dest_id = 0x3344;
	const uint32_t arg0 = 0x01234567ULL;
	const uint32_t arg1 = 0x12345678ULL;
	const uint32_t arg2 = 0x23456789ULL;
	const uint32_t arg3 = 0x3456789aULL;
	const uint32_t arg4 = 0x456789abULL;
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000070, ((uint32_t)source_id << 16) | dest_id, 0,
		       arg0, arg1, arg2, arg3, arg4, &svc_result);
	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_msg_send_direct_resp(source_id, dest_id, arg0, arg1, arg2,
					 arg3, arg4, &msg);
	}
}

TEST(ffa_api, ffa_mem_donate)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint64_t handle = 0;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;

	svc_result.a0 = 0x84000061;
	svc_result.a2 = (handle_result & 0xffffffffULL);
	svc_result.a3 = (handle_result >> 32);
	expect_ffa_svc(0x84000071, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	ffa_result result =
		ffa_mem_donate(total_length, fragment_length,
			       (void *)buffer_address, page_count, &handle);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGLONGS_EQUAL(handle_result, handle);
}

TEST(ffa_api, ffa_mem_donate_error)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint64_t handle = 0x1234567812345678;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;

	setup_error_response(-1);
	expect_ffa_svc(0x84000071, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	ffa_result result =
		ffa_mem_donate(total_length, fragment_length,
			       (void *)buffer_address, page_count, &handle);
	LONGS_EQUAL(-1, result);
	UNSIGNED_LONGLONGS_EQUAL(0, handle);
}

TEST(ffa_api, ffa_mem_donate_unknown_response)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint64_t handle = 0x1234567812345678;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000071, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_mem_donate(total_length, fragment_length,
			       (void *)buffer_address, page_count, &handle);
	}
}

TEST(ffa_api, ffa_mem_donate_rxtx)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	uint64_t handle = 0;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;

	svc_result.a0 = 0x84000061;
	svc_result.a2 = (handle_result & 0xffffffffULL);
	svc_result.a3 = (handle_result >> 32);
	expect_ffa_svc(0x84000071, total_length, fragment_length, 0, 0, 0, 0, 0,
		       &svc_result);

	ffa_result result =
		ffa_mem_donate_rxtx(total_length, fragment_length, &handle);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGLONGS_EQUAL(handle_result, handle);
}

TEST(ffa_api, ffa_mem_lend)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint64_t handle = 0;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;

	svc_result.a0 = 0x84000061;
	svc_result.a2 = (handle_result & 0xffffffffULL);
	svc_result.a3 = (handle_result >> 32);
	expect_ffa_svc(0x84000072, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	ffa_result result =
		ffa_mem_lend(total_length, fragment_length,
			     (void *)buffer_address, page_count, &handle);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGLONGS_EQUAL(handle_result, handle);
}

TEST(ffa_api, ffa_mem_lend_error)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint64_t handle = 0x1234567812345678;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;

	setup_error_response(-1);
	expect_ffa_svc(0x84000072, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	ffa_result result =
		ffa_mem_lend(total_length, fragment_length,
			     (void *)buffer_address, page_count, &handle);
	LONGS_EQUAL(-1, result);
	UNSIGNED_LONGLONGS_EQUAL(0, handle);
}

TEST(ffa_api, ffa_mem_lend_unknown_response)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint64_t handle = 0x1234567812345678;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000072, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_mem_lend(total_length, fragment_length,
			     (void *)buffer_address, page_count, &handle);
	}
}

TEST(ffa_api, ffa_mem_lend_rxtx)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	uint64_t handle = 0;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;

	svc_result.a0 = 0x84000061;
	svc_result.a2 = (handle_result & 0xffffffffULL);
	svc_result.a3 = (handle_result >> 32);
	expect_ffa_svc(0x84000072, total_length, fragment_length, 0, 0, 0, 0, 0,
		       &svc_result);

	ffa_result result =
		ffa_mem_lend_rxtx(total_length, fragment_length, &handle);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGLONGS_EQUAL(handle_result, handle);
}

TEST(ffa_api, ffa_mem_share)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint64_t handle = 0;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;

	svc_result.a0 = 0x84000061;
	svc_result.a2 = (handle_result & 0xffffffffULL);
	svc_result.a3 = (handle_result >> 32);
	expect_ffa_svc(0x84000073, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	ffa_result result =
		ffa_mem_share(total_length, fragment_length,
			      (void *)buffer_address, page_count, &handle);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGLONGS_EQUAL(handle_result, handle);
}

TEST(ffa_api, ffa_mem_share_error)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint64_t handle = 0x1234567812345678;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;

	setup_error_response(-1);
	expect_ffa_svc(0x84000073, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	ffa_result result =
		ffa_mem_share(total_length, fragment_length,
			      (void *)buffer_address, page_count, &handle);
	LONGS_EQUAL(-1, result);
	UNSIGNED_LONGLONGS_EQUAL(0, handle);
}

TEST(ffa_api, ffa_mem_share_unknown_response)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint64_t handle = 0x1234567812345678;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000073, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_mem_share(total_length, fragment_length,
			      (void *)buffer_address, page_count, &handle);
	}
}

TEST(ffa_api, ffa_mem_share_rxtx)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	uint64_t handle = 0;
	const uint64_t handle_result = 0xaabbccdd11223344ULL;

	svc_result.a0 = 0x84000061;
	svc_result.a2 = (handle_result & 0xffffffffULL);
	svc_result.a3 = (handle_result >> 32);
	expect_ffa_svc(0x84000073, total_length, fragment_length, 0, 0, 0, 0, 0,
		       &svc_result);

	ffa_result result =
		ffa_mem_share_rxtx(total_length, fragment_length, &handle);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGLONGS_EQUAL(handle_result, handle);
}

TEST(ffa_api, ffa_mem_retrieve_req)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint32_t resp_total_length = 0;
	uint32_t resp_fragment_length = 0;
	const uint32_t resp_total_length_result = 0xaabbccdd;
	const uint32_t resp_frament_length_result = 0x11223344;

	svc_result.a0 = 0x84000075;
	svc_result.a1 = resp_total_length_result;
	svc_result.a2 = resp_frament_length_result;
	expect_ffa_svc(0x84000074, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	ffa_result result =
		ffa_mem_retrieve_req(total_length, fragment_length,
				     (void *)buffer_address, page_count,
				     &resp_total_length, &resp_fragment_length);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGS_EQUAL(resp_total_length_result, resp_total_length);
	UNSIGNED_LONGS_EQUAL(resp_frament_length_result, resp_fragment_length);
}

TEST(ffa_api, ffa_mem_retrieve_req_error)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint32_t resp_total_length = 0;
	uint32_t resp_fragment_length = 0;
	const uint32_t resp_total_length_result = 0xaabbccdd;
	const uint32_t resp_frament_length_result = 0x11223344;

	setup_error_response(-1);
	expect_ffa_svc(0x84000074, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	ffa_result result =
		ffa_mem_retrieve_req(total_length, fragment_length,
				     (void *)buffer_address, page_count,
				     &resp_total_length, &resp_fragment_length);
	LONGS_EQUAL(-1, result);
	UNSIGNED_LONGS_EQUAL(0, resp_total_length);
	UNSIGNED_LONGS_EQUAL(0, resp_fragment_length);
}

TEST(ffa_api, ffa_mem_retrieve_req_unknown_response)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	const uint64_t buffer_address = 0x0123456789abcdefULL;
	const uint32_t page_count = 0x87654321;
	uint32_t resp_total_length = 0;
	uint32_t resp_fragment_length = 0;
	const uint32_t resp_total_length_result = 0xaabbccdd;
	const uint32_t resp_frament_length_result = 0x11223344;
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000074, total_length, fragment_length,
		       buffer_address, page_count, 0, 0, 0, &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_mem_retrieve_req(total_length, fragment_length,
				     (void *)buffer_address, page_count,
				     &resp_total_length, &resp_fragment_length);
	}
}

TEST(ffa_api, ffa_mem_retrieve_req_rxtx)
{
	const uint32_t total_length = 0xfedcba98UL;
	const uint32_t fragment_length = 0xffeeddcc;
	uint32_t resp_total_length = 0;
	uint32_t resp_fragment_length = 0;
	const uint32_t resp_total_length_result = 0xaabbccdd;
	const uint32_t resp_frament_length_result = 0x11223344;

	svc_result.a0 = 0x84000075;
	svc_result.a1 = resp_total_length_result;
	svc_result.a2 = resp_frament_length_result;
	expect_ffa_svc(0x84000074, total_length, fragment_length, 0, 0, 0, 0, 0,
		       &svc_result);

	ffa_result result =
		ffa_mem_retrieve_req_rxtx(total_length, fragment_length,
					  &resp_total_length,
					  &resp_fragment_length);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGS_EQUAL(resp_total_length_result, resp_total_length);
	UNSIGNED_LONGS_EQUAL(resp_frament_length_result, resp_fragment_length);
}

TEST(ffa_api, ffa_mem_relinquish)
{
	ffa_result result = FFA_OK;

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x84000076, 0, 0, 0, 0, 0, 0, 0, &svc_result);

	result = ffa_mem_relinquish();
	LONGS_EQUAL(FFA_OK, result);
}

TEST(ffa_api, ffa_mem_relinquish_error)
{
	ffa_result result = FFA_OK;

	setup_error_response(-1);
	expect_ffa_svc(0x84000076, 0, 0, 0, 0, 0, 0, 0, &svc_result);

	result = ffa_mem_relinquish();
	LONGS_EQUAL(-1, result);
}

TEST(ffa_api, ffa_mem_relinquish_unknown_response)
{
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000076, 0, 0, 0, 0, 0, 0, 0, &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_mem_relinquish();
	}
}

TEST(ffa_api, ffa_mem_reclaim)
{
	const uint64_t handle = 0x1234567812345678;
	const uint32_t flags = 0x87654321;

	svc_result.a0 = 0x84000061;
	expect_ffa_svc(0x84000077, (handle & 0xffffffffULL), handle >> 32,
		       flags, 0, 0, 0, 0, &svc_result);

	ffa_result result = ffa_mem_reclaim(handle, flags);
	LONGS_EQUAL(0, result);
}

TEST(ffa_api, ffa_mem_reclaimerror)
{
	const uint64_t handle = 0x1234567812345678;
	const uint32_t flags = 0x87654321;

	setup_error_response(-1);
	expect_ffa_svc(0x84000077, (handle & 0xffffffffULL), handle >> 32,
		       flags, 0, 0, 0, 0, &svc_result);

	ffa_result result = ffa_mem_reclaim(handle, flags);
	LONGS_EQUAL(-1, result);
}

TEST(ffa_api, ffa_mem_reclaim_unknown_response)
{
	const uint64_t handle = 0x1234567812345678;
	const uint32_t flags = 0x87654321;
	assert_environment_t assert_env;

	svc_result.a0 = 0x12345678;
	expect_ffa_svc(0x84000077, (handle & 0xffffffffULL), handle >> 32,
		       flags, 0, 0, 0, 0, &svc_result);

	if (SETUP_ASSERT_ENVIRONMENT(assert_env)) {
		ffa_mem_reclaim(handle, flags);
	}
}
