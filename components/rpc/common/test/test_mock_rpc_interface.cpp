// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <string.h>
#include "mock_rpc_interface.h"

TEST_GROUP(mock_rpc_interface)
{
	TEST_SETUP()
	{
		mock_rpc_interface_init();
		memset(&iface, 0x00, sizeof(iface));
	}

	TEST_TEARDOWN()
	{
		mock().checkExpectations();
		mock().removeAllComparatorsAndCopiers();
		mock().clear();
	}

	struct rpc_interface iface;
};

TEST(mock_rpc_interface, receive)
{
	rpc_status_t res = TS_RPC_ERROR_INTERNAL;
	struct call_req expected_req = { 0 };
	struct call_req req = { 0 };

	iface.context = (void *)1;
	iface.receive = mock_rpc_interface_receive;

	expected_req.caller_id = 0x01234567;
	expected_req.interface_id = 0x89abcdef;
	expected_req.opcode = 0xfedcba98;
	expected_req.encoding = 0x76543210;
	expected_req.opstatus = (rpc_opstatus_t)-1;

	expected_req.req_buf.size = 1;
	expected_req.req_buf.data_len = 2;
	expected_req.req_buf.data = (void *)3;

	expected_req.resp_buf.size = 4;
	expected_req.resp_buf.data_len = 5;
	expected_req.resp_buf.data = (void *)6;

	memcpy(&req, &expected_req, sizeof(req));
	req.opstatus = 0;
	req.resp_buf.data_len = 0;

	expect_mock_rpc_interface_receive(&iface, &expected_req, res);
	LONGS_EQUAL(res, mock_rpc_interface_receive(&iface, &req));

	UNSIGNED_LONGLONGS_EQUAL(expected_req.opstatus, req.opstatus);
	UNSIGNED_LONGLONGS_EQUAL(expected_req.resp_buf.data_len, req.resp_buf.data_len);
}
