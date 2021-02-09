// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 */

#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <stdint.h>
#include <string.h>
#include "mock_ffa_api.h"
#include "../include/sp_messaging.h"

#define SP_MSG_ARG_OFFSET (1)

TEST_GROUP(sp_messaging)
{
	TEST_SETUP()
	{
		memset(&req, 0x00, sizeof(req));
		memset(&resp, 0x00, sizeof(resp));
		memset(&ffa_msg, 0x00, sizeof(ffa_msg));
	}

	TEST_TEARDOWN()
	{
		mock().checkExpectations();
		mock().clear();
	}

	void copy_sp_to_ffa_args(const uint32_t sp_args[], uint32_t ffa_args[])
	{
		int i = 0;

		for (i = 0; i < SP_MSG_ARG_COUNT; i++) {
			ffa_args[i + SP_MSG_ARG_OFFSET] = sp_args[i];
		}
	}

	void fill_ffa_msg(struct ffa_direct_msg * msg)
	{
		int i = 0;

		msg->function_id = FFA_MSG_SEND_DIRECT_REQ_32;
		msg->source_id = source_id;
		msg->destination_id = dest_id;

		msg->args[0] = 0;
		for (i = 0; i < SP_MSG_ARG_COUNT; i++) {
			msg->args[i + SP_MSG_ARG_OFFSET] = args[i];
		}
	}

	void fill_sp_msg(struct sp_msg * msg)
	{
		int i = 0;

		msg->source_id = source_id;
		msg->destination_id = dest_id;
		for (i = 0; i < SP_MSG_ARG_COUNT; i++) {
			msg->args[i] = args[i + SP_MSG_ARG_OFFSET];
		}
	}

	void ffa_and_sp_msg_equal(const struct ffa_direct_msg *ffa_msg,
				  const struct sp_msg *sp_msg)
	{
		int i = 0;

		UNSIGNED_LONGS_EQUAL(ffa_msg->source_id, sp_msg->source_id);
		UNSIGNED_LONGS_EQUAL(ffa_msg->destination_id,
				     sp_msg->destination_id);
		for (i = 0; i < SP_MSG_ARG_COUNT; i++) {
			UNSIGNED_LONGS_EQUAL(
				ffa_msg->args[i + SP_MSG_ARG_OFFSET],
				sp_msg->args[i]);
		}
	}

	struct sp_msg req;
	struct sp_msg resp;
	struct ffa_direct_msg ffa_msg;

	const uint16_t source_id = 0x1234;
	const uint16_t dest_id = 0x5678;
	const uint32_t args[SP_MSG_ARG_COUNT] = { 0x01234567, 0x12345678,
						  0x23456789, 0x3456789a };
	const sp_result result = -1;
	const sp_msg empty_sp_msg = (const sp_msg){ 0 };
};

TEST(sp_messaging, sp_msg_wait_null_msg)
{
	LONGS_EQUAL(SP_RESULT_INVALID_PARAMETERS, sp_msg_wait(NULL));
}

TEST(sp_messaging, sp_msg_wait_ffa_error)
{
	ffa_result result = FFA_ABORTED;

	expect_ffa_msg_wait(&ffa_msg, result);

	LONGS_EQUAL(SP_RESULT_FFA(result), sp_msg_wait(&req));
	MEMCMP_EQUAL(&empty_sp_msg, &req, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_wait)
{
	fill_ffa_msg(&ffa_msg);
	expect_ffa_msg_wait(&ffa_msg, FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_wait(&req));
	ffa_and_sp_msg_equal(&ffa_msg, &req);
}

TEST(sp_messaging, sp_msg_send_direct_req_resp_null)
{
	LONGS_EQUAL(SP_RESULT_INVALID_PARAMETERS,
		    sp_msg_send_direct_req(&req, NULL));
}

TEST(sp_messaging, sp_msg_send_direct_req_req_null)
{
	memset(&resp, 0x5a, sizeof(resp));
	LONGS_EQUAL(SP_RESULT_INVALID_PARAMETERS,
		    sp_msg_send_direct_req(NULL, &resp));
	MEMCMP_EQUAL(&empty_sp_msg, &resp, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_send_direct_req_ffa_error)
{
	ffa_result result = FFA_ABORTED;
	uint32_t expected_ffa_args[5] = { 0 };

	fill_sp_msg(&req);
	memset(&resp, 0x5a, sizeof(resp));
	copy_sp_to_ffa_args(req.args, expected_ffa_args);
	expect_ffa_msg_send_direct_req(
		req.source_id, req.destination_id, expected_ffa_args[0],
		expected_ffa_args[1], expected_ffa_args[2],
		expected_ffa_args[3], expected_ffa_args[4], &ffa_msg, result);

	LONGS_EQUAL(SP_RESULT_FFA(result), sp_msg_send_direct_req(&req, &resp));
	MEMCMP_EQUAL(&empty_sp_msg, &resp, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_send_direct_req_msg)
{
	uint32_t expected_ffa_args[5] = { 0 };

	fill_sp_msg(&req);
	fill_ffa_msg(&ffa_msg);
	copy_sp_to_ffa_args(req.args, expected_ffa_args);
	expect_ffa_msg_send_direct_req(
		req.source_id, req.destination_id, expected_ffa_args[0],
		expected_ffa_args[1], expected_ffa_args[2],
		expected_ffa_args[3], expected_ffa_args[4], &ffa_msg, FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_direct_req(&req, &resp));
	ffa_and_sp_msg_equal(&ffa_msg, &resp);
}

TEST(sp_messaging, sp_msg_send_direct_req_success)
{
	uint32_t expected_ffa_args[5] = { 0 };

	fill_sp_msg(&req);
	ffa_msg.function_id = FFA_SUCCESS_32;
	copy_sp_to_ffa_args(req.args, expected_ffa_args);
	expect_ffa_msg_send_direct_req(
		req.source_id, req.destination_id, expected_ffa_args[0],
		expected_ffa_args[1], expected_ffa_args[2],
		expected_ffa_args[3], expected_ffa_args[4], &ffa_msg, FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_direct_req(&req, &resp));
	MEMCMP_EQUAL(&empty_sp_msg, &resp, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_send_direct_resp_req_null)
{
	LONGS_EQUAL(SP_RESULT_INVALID_PARAMETERS,
		    sp_msg_send_direct_resp(&resp, NULL));
}

TEST(sp_messaging, sp_msg_send_direct_resp_resp_null)
{
	memset(&req, 0x5a, sizeof(req));
	LONGS_EQUAL(SP_RESULT_INVALID_PARAMETERS,
		    sp_msg_send_direct_resp(NULL, &req));
	MEMCMP_EQUAL(&empty_sp_msg, &req, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_send_direct_resp_ffa_error)
{
	ffa_result result = FFA_ABORTED;
	uint32_t expected_ffa_args[5] = { 0 };

	fill_sp_msg(&resp);
	memset(&req, 0x5a, sizeof(req));
	copy_sp_to_ffa_args(resp.args, expected_ffa_args);
	expect_ffa_msg_send_direct_resp(
		resp.source_id, resp.destination_id, expected_ffa_args[0],
		expected_ffa_args[1], expected_ffa_args[2],
		expected_ffa_args[3], expected_ffa_args[4], &ffa_msg, result);

	LONGS_EQUAL(SP_RESULT_FFA(result),
		    sp_msg_send_direct_resp(&resp, &req));
	MEMCMP_EQUAL(&empty_sp_msg, &req, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_send_direct_resp_msg)
{
	uint32_t expected_ffa_args[5] = { 0 };

	fill_sp_msg(&resp);
	fill_ffa_msg(&ffa_msg);
	copy_sp_to_ffa_args(resp.args, expected_ffa_args);
	expect_ffa_msg_send_direct_resp(
		resp.source_id, resp.destination_id, expected_ffa_args[0],
		expected_ffa_args[1], expected_ffa_args[2],
		expected_ffa_args[3], expected_ffa_args[4], &ffa_msg, FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_direct_resp(&resp, &req));
	ffa_and_sp_msg_equal(&ffa_msg, &req);
}

TEST(sp_messaging, sp_msg_send_direct_resp_success)
{
	uint32_t expected_ffa_args[5] = { 0 };

	fill_sp_msg(&req);
	fill_sp_msg(&resp);
	ffa_msg.function_id = FFA_SUCCESS_32;
	copy_sp_to_ffa_args(resp.args, expected_ffa_args);
	expect_ffa_msg_send_direct_resp(
		resp.source_id, resp.destination_id, expected_ffa_args[0],
		expected_ffa_args[1], expected_ffa_args[2],
		expected_ffa_args[3], expected_ffa_args[4], &ffa_msg, FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_direct_resp(&resp, &req));
	MEMCMP_EQUAL(&empty_sp_msg, &req, sizeof(empty_sp_msg));
}
