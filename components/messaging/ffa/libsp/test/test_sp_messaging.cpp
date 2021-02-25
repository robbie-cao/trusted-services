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

#if FFA_DIRECT_MSG_ROUTING_EXTENSION
#define ROUTING_EXT_RC_BIT BIT(0)
#define ROUTING_EXT_ERR_BIT BIT(1)
#endif

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

#if FFA_DIRECT_MSG_ROUTING_EXTENSION
	void wait_and_receive_request(uint16_t source_id, uint16_t dest_id)
	{
		struct ffa_direct_msg expected_ffa_req = { 0 };
		struct sp_msg req = { 0 };

		fill_ffa_msg(&expected_ffa_req);
		expected_ffa_req.source_id = source_id;
		expected_ffa_req.destination_id = dest_id;
		expect_ffa_msg_wait(&expected_ffa_req, FFA_OK);

		LONGS_EQUAL(SP_RESULT_OK, sp_msg_wait(&req));
		ffa_and_sp_msg_equal(&expected_ffa_req, &req);
	}
#endif

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

#if FFA_DIRECT_MSG_ROUTING_EXTENSION
TEST(sp_messaging, sp_msg_wait_deny_rc_failure)
{
	struct ffa_direct_msg rc_msg = { 0 };
	ffa_result result = FFA_ABORTED;

	fill_ffa_msg(&rc_msg);
	rc_msg.args[0] = ROUTING_EXT_RC_BIT;
	expect_ffa_msg_wait(&rc_msg, FFA_OK);

	fill_ffa_msg(&ffa_msg);
	expect_ffa_msg_send_direct_resp(
		rc_msg.destination_id, rc_msg.source_id,
		ROUTING_EXT_RC_BIT | ROUTING_EXT_ERR_BIT,
		SP_RESULT_FFA(FFA_DENIED), 0, 0, 0, &ffa_msg, result);

	LONGS_EQUAL(SP_RESULT_FFA(result), sp_msg_wait(&req));
	MEMCMP_EQUAL(&empty_sp_msg, &req, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_wait_deny_rc)
{
	struct ffa_direct_msg rc_msg = { 0 };

	fill_ffa_msg(&rc_msg);
	rc_msg.args[0] = ROUTING_EXT_RC_BIT;
	expect_ffa_msg_wait(&rc_msg, FFA_OK);

	fill_ffa_msg(&ffa_msg);
	expect_ffa_msg_send_direct_resp(
		rc_msg.destination_id, rc_msg.source_id,
		ROUTING_EXT_RC_BIT | ROUTING_EXT_ERR_BIT,
		SP_RESULT_FFA(FFA_DENIED), 0, 0, 0, &ffa_msg, FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_wait(&req));
	ffa_and_sp_msg_equal(&ffa_msg, &req);
}
#endif

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

#if FFA_DIRECT_MSG_ROUTING_EXTENSION
TEST(sp_messaging, sp_msg_send_direct_req_rc_forwarding_success)
{
	const uint16_t root_id = 1;
	const uint16_t own_id = 2;
	const uint16_t rc_root_id = 3;
	ffa_direct_msg req = { 0 };
	ffa_direct_msg rc_req = { 0 };
	ffa_direct_msg rc_resp = { 0 };
	ffa_direct_msg resp = { 0 };
	sp_msg sp_req = { 0 };
	sp_msg sp_resp = { 0 };

	fill_sp_msg(&sp_req);
	sp_req.source_id = own_id;
	sp_req.destination_id = rc_root_id;

	req.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	req.source_id = own_id;
	req.destination_id = rc_root_id;
	copy_sp_to_ffa_args(sp_req.args, req.args);

	fill_ffa_msg(&rc_req);
	rc_req.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	rc_req.source_id = rc_root_id;
	rc_req.destination_id = own_id;
	rc_req.args[0] = ROUTING_EXT_RC_BIT;

	fill_ffa_msg(&rc_resp);
	rc_resp.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	rc_resp.source_id = root_id;
	rc_resp.destination_id = own_id;
	rc_resp.args[0] = ROUTING_EXT_RC_BIT;

	fill_sp_msg(&sp_resp);
	sp_resp.source_id = rc_root_id;
	sp_resp.destination_id = own_id;

	resp.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	resp.source_id = rc_root_id;
	resp.destination_id = own_id;
	copy_sp_to_ffa_args(sp_resp.args, resp.args);

	/* Initial request to current SP to set own_id */
	wait_and_receive_request(root_id, own_id);

	/* Sending request and receiving RC request from RC root */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, 0, req.args[1],
				       req.args[2], req.args[3], req.args[4],
				       &rc_req, FFA_OK);

	/* Forwarding RC request to root and receiving RC response */
	expect_ffa_msg_send_direct_resp(own_id, root_id, rc_req.args[0],
					rc_req.args[1], rc_req.args[2],
					rc_req.args[3], rc_req.args[4],
					&rc_resp, FFA_OK);

	/* Fowarding RC response to RC root and receiving response */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, rc_resp.args[0],
				       rc_resp.args[1], rc_resp.args[2],
				       rc_resp.args[3], rc_resp.args[4], &resp,
				       FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_direct_req(&sp_req, &sp_resp));
	ffa_and_sp_msg_equal(&resp, &sp_resp);
}

TEST(sp_messaging, sp_msg_send_direct_req_rc_error)
{
	const uint16_t root_id = 1;
	const uint16_t own_id = 2;
	const uint16_t rc_root_id = 3;
	ffa_direct_msg req = { 0 };
	ffa_direct_msg rc_err = { 0 };
	sp_msg sp_req = { 0 };
	sp_msg sp_resp = { 0 };

	fill_sp_msg(&sp_req);
	sp_req.source_id = own_id;
	sp_req.destination_id = rc_root_id;

	req.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	req.source_id = own_id;
	req.destination_id = rc_root_id;
	copy_sp_to_ffa_args(sp_req.args, req.args);

	fill_ffa_msg(&rc_err);
	rc_err.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	rc_err.source_id = rc_root_id;
	rc_err.destination_id = own_id;
	rc_err.args[0] = ROUTING_EXT_RC_BIT | ROUTING_EXT_ERR_BIT;
	rc_err.args[1] = result;

	/* Initial request to current SP to set own_id */
	wait_and_receive_request(root_id, own_id);

	/* Sending request and receiving RC request from RC root */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, 0, req.args[1],
				       req.args[2], req.args[3], req.args[4],
				       &rc_err, FFA_OK);

	LONGS_EQUAL(SP_RESULT_FFA(result),
		    sp_msg_send_direct_req(&sp_req, &sp_resp));
	MEMCMP_EQUAL(&empty_sp_msg, &sp_resp, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_send_direct_req_rc_forwarding_success_deny_request)
{
	const uint16_t root_id = 1;
	const uint16_t own_id = 2;
	const uint16_t rc_root_id = 3;
	ffa_direct_msg req = { 0 };
	ffa_direct_msg rc_req = { 0 };
	ffa_direct_msg request_to_deny = { 0 };
	ffa_direct_msg rc_resp = { 0 };
	ffa_direct_msg resp = { 0 };
	sp_msg sp_req = { 0 };
	sp_msg sp_resp = { 0 };

	fill_sp_msg(&sp_req);
	sp_req.source_id = own_id;
	sp_req.destination_id = rc_root_id;

	req.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	req.source_id = own_id;
	req.destination_id = rc_root_id;
	copy_sp_to_ffa_args(sp_req.args, req.args);

	fill_ffa_msg(&rc_req);
	rc_req.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	rc_req.source_id = rc_root_id;
	rc_req.destination_id = own_id;
	rc_req.args[0] = ROUTING_EXT_RC_BIT;

	request_to_deny.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	request_to_deny.source_id = root_id;
	request_to_deny.destination_id = own_id;
	request_to_deny.args[0] = 0;

	fill_ffa_msg(&rc_resp);
	rc_resp.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	rc_resp.source_id = root_id;
	rc_resp.destination_id = own_id;
	rc_resp.args[0] = ROUTING_EXT_RC_BIT;

	fill_sp_msg(&sp_resp);
	sp_resp.source_id = rc_root_id;
	sp_resp.destination_id = own_id;

	resp.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	resp.source_id = rc_root_id;
	resp.destination_id = own_id;
	copy_sp_to_ffa_args(sp_resp.args, resp.args);

	/* Initial request to current SP to set own_id */
	wait_and_receive_request(root_id, own_id);

	/* Sending request and receiving RC request from RC root */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, 0, req.args[1],
				       req.args[2], req.args[3], req.args[4],
				       &rc_req, FFA_OK);

	/* Forwarding RC request to root and receiving a request to deny */
	expect_ffa_msg_send_direct_resp(own_id, root_id, rc_req.args[0],
					rc_req.args[1], rc_req.args[2],
					rc_req.args[3], rc_req.args[4],
					&request_to_deny, FFA_OK);

	/* Sending error to root and receiving RC response */
	expect_ffa_msg_send_direct_resp(
		own_id, root_id, ROUTING_EXT_RC_BIT | ROUTING_EXT_ERR_BIT,
		SP_RESULT_FFA(FFA_BUSY), 0, 0, 0, &rc_resp, FFA_OK);

	/* Fowarding RC response to RC root and receiving response */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, rc_resp.args[0],
				       rc_resp.args[1], rc_resp.args[2],
				       rc_resp.args[3], rc_resp.args[4], &resp,
				       FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_direct_req(&sp_req, &sp_resp));
	ffa_and_sp_msg_equal(&resp, &sp_resp);
}

TEST(sp_messaging, sp_msg_send_direct_req_rc_forwarding_success_invalid_req_src)
{
	const uint16_t root_id = 1;
	const uint16_t own_id = 2;
	const uint16_t rc_root_id = 3;
	ffa_direct_msg req = { 0 };
	ffa_direct_msg rc_req = { 0 };
	ffa_direct_msg request_to_deny = { 0 };
	ffa_direct_msg rc_resp = { 0 };
	ffa_direct_msg resp = { 0 };
	sp_msg sp_req = { 0 };
	sp_msg sp_resp = { 0 };

	fill_sp_msg(&sp_req);
	sp_req.source_id = own_id;
	sp_req.destination_id = rc_root_id;

	req.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	req.source_id = own_id;
	req.destination_id = rc_root_id;
	copy_sp_to_ffa_args(sp_req.args, req.args);

	fill_ffa_msg(&rc_req);
	rc_req.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	rc_req.source_id = rc_root_id;
	rc_req.destination_id = own_id;
	rc_req.args[0] = ROUTING_EXT_RC_BIT;

	request_to_deny.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	/* This source ID should be denied in the current state. */
	request_to_deny.source_id = rc_root_id;
	request_to_deny.destination_id = own_id;
	request_to_deny.args[0] = ROUTING_EXT_RC_BIT;

	fill_ffa_msg(&rc_resp);
	rc_resp.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	rc_resp.source_id = root_id;
	rc_resp.destination_id = own_id;
	rc_resp.args[0] = ROUTING_EXT_RC_BIT;

	fill_sp_msg(&sp_resp);
	sp_resp.source_id = rc_root_id;
	sp_resp.destination_id = own_id;

	resp.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	resp.source_id = rc_root_id;
	resp.destination_id = own_id;
	copy_sp_to_ffa_args(sp_resp.args, resp.args);

	/* Initial request to current SP to set own_id */
	wait_and_receive_request(root_id, own_id);

	/* Sending request and receiving RC request from RC root */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, 0, req.args[1],
				       req.args[2], req.args[3], req.args[4],
				       &rc_req, FFA_OK);

	/* Forwarding RC request to root and receiving RC response */
	expect_ffa_msg_send_direct_resp(own_id, root_id, rc_req.args[0],
					rc_req.args[1], rc_req.args[2],
					rc_req.args[3], rc_req.args[4],
					&request_to_deny, FFA_OK);

	/* Sending error to root and receiving RC response */
	expect_ffa_msg_send_direct_resp(
		own_id, rc_root_id, ROUTING_EXT_ERR_BIT | ROUTING_EXT_RC_BIT,
		SP_RESULT_FFA(FFA_BUSY), 0, 0, 0, &rc_resp, FFA_OK);

	/* Fowarding RC response to RC root and receiving response */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, rc_resp.args[0],
				       rc_resp.args[1], rc_resp.args[2],
				       rc_resp.args[3], rc_resp.args[4], &resp,
				       FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_direct_req(&sp_req, &sp_resp));
	ffa_and_sp_msg_equal(&resp, &sp_resp);
}

TEST(sp_messaging, sp_msg_send_direct_req_deny_fail_wait_success)
{
	const uint16_t root_id = 1;
	const uint16_t own_id = 2;
	const uint16_t rc_root_id = 3;
	ffa_direct_msg req = { 0 };
	ffa_direct_msg rc_req = { 0 };
	ffa_direct_msg request_to_deny = { 0 };
	ffa_direct_msg rc_resp = { 0 };
	ffa_direct_msg resp = { 0 };
	sp_msg sp_req = { 0 };
	sp_msg sp_resp = { 0 };

	fill_sp_msg(&sp_req);
	sp_req.source_id = own_id;
	sp_req.destination_id = rc_root_id;

	req.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	req.source_id = own_id;
	req.destination_id = rc_root_id;
	copy_sp_to_ffa_args(sp_req.args, req.args);

	fill_ffa_msg(&rc_req);
	rc_req.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	rc_req.source_id = rc_root_id;
	rc_req.destination_id = own_id;
	rc_req.args[0] = ROUTING_EXT_RC_BIT;

	request_to_deny.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	/* This source ID should be denied in the current state. */
	request_to_deny.source_id = rc_root_id;
	request_to_deny.destination_id = own_id;
	request_to_deny.args[0] = ROUTING_EXT_RC_BIT;

	fill_ffa_msg(&rc_resp);
	rc_resp.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	rc_resp.source_id = root_id;
	rc_resp.destination_id = own_id;
	rc_resp.args[0] = ROUTING_EXT_RC_BIT;

	fill_sp_msg(&sp_resp);
	sp_resp.source_id = rc_root_id;
	sp_resp.destination_id = own_id;

	resp.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	resp.source_id = rc_root_id;
	resp.destination_id = own_id;
	copy_sp_to_ffa_args(sp_resp.args, resp.args);

	/* Initial request to current SP to set own_id */
	wait_and_receive_request(root_id, own_id);

	/* Sending request and receiving RC request from RC root */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, 0, req.args[1],
				       req.args[2], req.args[3], req.args[4],
				       &rc_req, FFA_OK);

	/* Forwarding RC request to root and receiving RC response */
	expect_ffa_msg_send_direct_resp(own_id, root_id, rc_req.args[0],
					rc_req.args[1], rc_req.args[2],
					rc_req.args[3], rc_req.args[4],
					&request_to_deny, FFA_OK);

	/* Sending error to root which fails */
	expect_ffa_msg_send_direct_resp(
		own_id, rc_root_id, (ROUTING_EXT_ERR_BIT | ROUTING_EXT_RC_BIT),
		SP_RESULT_FFA(FFA_BUSY), 0, 0, 0, &rc_resp, FFA_DENIED);

	/* Sending FFA_MSG_WAIT as we are still waiting for the RC response */
	expect_ffa_msg_wait(&rc_resp, FFA_OK);

	/* Fowarding RC response to RC root and receiving response */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, rc_resp.args[0],
				       rc_resp.args[1], rc_resp.args[2],
				       rc_resp.args[3], rc_resp.args[4], &resp,
				       FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_direct_req(&sp_req, &sp_resp));
	ffa_and_sp_msg_equal(&resp, &sp_resp);
}

TEST(sp_messaging, sp_msg_send_direct_req_deny_fail_wait_fail_forwarding)
{
	const uint16_t root_id = 1;
	const uint16_t own_id = 2;
	const uint16_t rc_root_id = 3;
	ffa_direct_msg req = { 0 };
	ffa_direct_msg rc_req = { 0 };
	ffa_direct_msg request_to_deny = { 0 };
	ffa_direct_msg rc_resp = { 0 };
	ffa_direct_msg resp = { 0 };
	sp_msg sp_req = { 0 };
	sp_msg sp_resp = { 0 };

	fill_sp_msg(&sp_req);
	sp_req.source_id = own_id;
	sp_req.destination_id = rc_root_id;

	req.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	req.source_id = own_id;
	req.destination_id = rc_root_id;
	copy_sp_to_ffa_args(sp_req.args, req.args);

	fill_ffa_msg(&rc_req);
	rc_req.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	rc_req.source_id = rc_root_id;
	rc_req.destination_id = own_id;
	rc_req.args[0] = ROUTING_EXT_RC_BIT;

	request_to_deny.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	/* This source ID should be denied in the current state. */
	request_to_deny.source_id = rc_root_id;
	request_to_deny.destination_id = own_id;
	request_to_deny.args[0] = ROUTING_EXT_RC_BIT;

	fill_ffa_msg(&rc_resp);
	rc_resp.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	rc_resp.source_id = root_id;
	rc_resp.destination_id = own_id;
	rc_resp.args[0] = ROUTING_EXT_RC_BIT;

	fill_sp_msg(&sp_resp);
	sp_resp.source_id = rc_root_id;
	sp_resp.destination_id = own_id;

	resp.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	resp.source_id = rc_root_id;
	resp.destination_id = own_id;
	copy_sp_to_ffa_args(sp_resp.args, resp.args);

	/* Initial request to current SP to set own_id */
	wait_and_receive_request(root_id, own_id);

	/* Sending request and receiving RC request from RC root */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, 0, req.args[1],
				       req.args[2], req.args[3], req.args[4],
				       &rc_req, FFA_OK);

	/* Forwarding RC request to root and receiving RC response */
	expect_ffa_msg_send_direct_resp(own_id, root_id, rc_req.args[0],
					rc_req.args[1], rc_req.args[2],
					rc_req.args[3], rc_req.args[4],
					&request_to_deny, FFA_OK);

	/* Sending error to root which fails */
	expect_ffa_msg_send_direct_resp(
		own_id, rc_root_id, ROUTING_EXT_ERR_BIT | ROUTING_EXT_RC_BIT,
		SP_RESULT_FFA(FFA_BUSY), 0, 0, 0, &rc_resp, FFA_DENIED);

	/* Sending FFA_MSG_WAIT as we are still waiting for the RC response */
	expect_ffa_msg_wait(&rc_resp, result);

	/* Fowarding RC error as FFA_MSG_WAIT failed  */
	expect_ffa_msg_send_direct_req(
		own_id, rc_root_id, (ROUTING_EXT_RC_BIT | ROUTING_EXT_ERR_BIT),
		result, 0, 0, 0, &resp, FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_direct_req(&sp_req, &sp_resp));
	ffa_and_sp_msg_equal(&resp, &sp_resp);
}

TEST(sp_messaging, sp_msg_send_direct_req_rc_return_rc_error_msg)
{
	const uint16_t root_id = 1;
	const uint16_t own_id = 2;
	const uint16_t rc_root_id = 3;
	ffa_direct_msg req = { 0 };
	ffa_direct_msg rc_req = { 0 };
	ffa_direct_msg rc_resp = { 0 };
	ffa_direct_msg resp = { 0 };
	sp_msg sp_req = { 0 };
	sp_msg sp_resp = { 0 };
	ffa_result result = FFA_ABORTED;

	fill_sp_msg(&sp_req);
	sp_req.source_id = own_id;
	sp_req.destination_id = rc_root_id;

	req.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	req.source_id = own_id;
	req.destination_id = rc_root_id;
	copy_sp_to_ffa_args(sp_req.args, req.args);

	fill_ffa_msg(&rc_req);
	rc_req.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	rc_req.source_id = rc_root_id;
	rc_req.destination_id = own_id;
	rc_req.args[0] = ROUTING_EXT_RC_BIT;

	fill_ffa_msg(&rc_resp);
	rc_resp.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	rc_resp.source_id = root_id;
	rc_resp.destination_id = own_id;
	rc_resp.args[0] = ROUTING_EXT_RC_BIT;

	fill_sp_msg(&sp_resp);
	sp_resp.source_id = rc_root_id;
	sp_resp.destination_id = own_id;

	resp.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	resp.source_id = rc_root_id;
	resp.destination_id = own_id;
	copy_sp_to_ffa_args(sp_resp.args, resp.args);

	/* Initial request to current SP to set own_id */
	wait_and_receive_request(root_id, own_id);

	/* Sending request and receiving RC request from RC root */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, 0, req.args[1],
				       req.args[2], req.args[3], req.args[4],
				       &rc_req, FFA_OK);

	/* Forwarding RC request to root and receiving RC response */
	expect_ffa_msg_send_direct_resp(own_id, root_id, rc_req.args[0],
					rc_req.args[1], rc_req.args[2],
					rc_req.args[3], rc_req.args[4],
					&rc_resp, result);

	/* Fowarding RC error to RC root and receiving response */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id,
				       ROUTING_EXT_RC_BIT | ROUTING_EXT_ERR_BIT,
				       SP_RESULT_FFA(result), 0, 0, 0, &resp,
				       FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_direct_req(&sp_req, &sp_resp));
	ffa_and_sp_msg_equal(&resp, &sp_resp);
}

TEST(sp_messaging, sp_msg_send_direct_req_rc_return_resp_fail)
{
	const uint16_t root_id = 1;
	const uint16_t own_id = 2;
	const uint16_t rc_root_id = 3;
	ffa_direct_msg req = { 0 };
	ffa_direct_msg rc_req = { 0 };
	ffa_direct_msg rc_resp = { 0 };
	ffa_direct_msg resp = { 0 };
	sp_msg sp_req = { 0 };
	sp_msg sp_resp = { 0 };
	ffa_result result = FFA_ABORTED;

	fill_sp_msg(&sp_req);
	sp_req.source_id = own_id;
	sp_req.destination_id = rc_root_id;

	req.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	req.source_id = own_id;
	req.destination_id = rc_root_id;
	copy_sp_to_ffa_args(sp_req.args, req.args);

	fill_ffa_msg(&rc_req);
	rc_req.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	rc_req.source_id = rc_root_id;
	rc_req.destination_id = own_id;
	rc_req.args[0] = ROUTING_EXT_RC_BIT;

	fill_ffa_msg(&rc_resp);
	rc_resp.function_id = FFA_MSG_SEND_DIRECT_REQ_32;
	rc_resp.source_id = root_id;
	rc_resp.destination_id = own_id;
	rc_resp.args[0] = ROUTING_EXT_RC_BIT;

	fill_sp_msg(&sp_resp);
	sp_resp.source_id = rc_root_id;
	sp_resp.destination_id = own_id;

	resp.function_id = FFA_MSG_SEND_DIRECT_RESP_32;
	resp.source_id = rc_root_id;
	resp.destination_id = own_id;
	copy_sp_to_ffa_args(sp_resp.args, resp.args);

	/* Initial request to current SP to set own_id */
	wait_and_receive_request(root_id, own_id);

	/* Sending request and receiving RC request from RC root */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, 0, req.args[1],
				       req.args[2], req.args[3], req.args[4],
				       &rc_req, FFA_OK);

	/* Forwarding RC request to root and receiving RC response */
	expect_ffa_msg_send_direct_resp(own_id, root_id, rc_req.args[0],
					rc_req.args[1], rc_req.args[2],
					rc_req.args[3], rc_req.args[4],
					&rc_resp, FFA_OK);

	/* Fowarding RC response to RC root and receiving response */
	expect_ffa_msg_send_direct_req(own_id, rc_root_id, rc_resp.args[0],
				       rc_resp.args[1], rc_resp.args[2],
				       rc_resp.args[3], rc_resp.args[4], &resp,
				       result);

	LONGS_EQUAL(SP_RESULT_FFA(result),
		    sp_msg_send_direct_req(&sp_req, &sp_resp));
	MEMCMP_EQUAL(&empty_sp_msg, &sp_resp, sizeof(empty_sp_msg));
}
#endif

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

#if FFA_DIRECT_MSG_ROUTING_EXTENSION
TEST(sp_messaging, sp_msg_send_direct_resp_deny_rc_failure)
{
	uint32_t expected_ffa_args[5] = { 0 };
	struct ffa_direct_msg rc_msg = { 0 };

	fill_sp_msg(&resp);

	fill_ffa_msg(&rc_msg);
	rc_msg.args[0] = ROUTING_EXT_RC_BIT;

	fill_ffa_msg(&ffa_msg);
	copy_sp_to_ffa_args(resp.args, expected_ffa_args);

	expect_ffa_msg_send_direct_resp(
		resp.source_id, resp.destination_id, expected_ffa_args[0],
		expected_ffa_args[1], expected_ffa_args[2],
		expected_ffa_args[3], expected_ffa_args[4], &rc_msg, FFA_OK);

	expect_ffa_msg_send_direct_resp(
		rc_msg.destination_id, rc_msg.source_id,
		ROUTING_EXT_RC_BIT | ROUTING_EXT_ERR_BIT,
		SP_RESULT_FFA(FFA_DENIED), 0, 0, 0, &ffa_msg, result);

	LONGS_EQUAL(SP_RESULT_FFA(result),
		    sp_msg_send_direct_resp(&resp, &req));
	MEMCMP_EQUAL(&empty_sp_msg, &req, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_send_direct_resp_deny_rc)
{
	uint32_t expected_ffa_args[5] = { 0 };
	struct ffa_direct_msg rc_msg = { 0 };

	fill_sp_msg(&resp);

	fill_ffa_msg(&rc_msg);
	rc_msg.args[0] = ROUTING_EXT_RC_BIT;

	fill_ffa_msg(&ffa_msg);
	copy_sp_to_ffa_args(resp.args, expected_ffa_args);

	expect_ffa_msg_send_direct_resp(resp.source_id, resp.destination_id, 0,
					expected_ffa_args[1],
					expected_ffa_args[2],
					expected_ffa_args[3],
					expected_ffa_args[4], &rc_msg, FFA_OK);

	expect_ffa_msg_send_direct_resp(
		rc_msg.destination_id, rc_msg.source_id,
		ROUTING_EXT_RC_BIT | ROUTING_EXT_ERR_BIT,
		SP_RESULT_FFA(FFA_DENIED), 0, 0, 0, &ffa_msg, FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_direct_resp(&resp, &req));
	ffa_and_sp_msg_equal(&ffa_msg, &req);
}

TEST(sp_messaging, sp_msg_send_rc_req_req_null)
{
	LONGS_EQUAL(SP_RESULT_INVALID_PARAMETERS,
		    sp_msg_send_rc_req(&req, NULL));
}

TEST(sp_messaging, sp_msg_send_rc_req_resp_null)
{
	memset(&resp, 0x5a, sizeof(resp));
	LONGS_EQUAL(SP_RESULT_INVALID_PARAMETERS,
		    sp_msg_send_rc_req(NULL, &resp));
	MEMCMP_EQUAL(&empty_sp_msg, &resp, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_send_rc_req_ffa_error)
{
	ffa_result result = FFA_ABORTED;

	fill_sp_msg(&resp);
	memset(&req, 0x5a, sizeof(req));
	fill_ffa_msg(&ffa_msg);

	expect_ffa_msg_send_direct_resp(req.source_id, req.destination_id,
					ROUTING_EXT_RC_BIT, req.args[0],
					req.args[1], req.args[2], req.args[3],
					&ffa_msg, result);

	LONGS_EQUAL(SP_RESULT_FFA(result), sp_msg_send_rc_req(&req, &resp));
	MEMCMP_EQUAL(&empty_sp_msg, &resp, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_send_rc_req_deny_fail_wait_fail)
{
	const uint16_t root_id = 1;
	const uint16_t own_id = 2;

	wait_and_receive_request(root_id, own_id);

	fill_sp_msg(&req);
	req.source_id = own_id;
	req.destination_id = root_id;

	fill_ffa_msg(&ffa_msg);
	ffa_msg.source_id = root_id;
	ffa_msg.destination_id = own_id;
	/* Should be RC message so it will be denied */
	ffa_msg.args[0] = 0;

	expect_ffa_msg_send_direct_resp(req.source_id, req.destination_id,
					ROUTING_EXT_RC_BIT, req.args[0],
					req.args[1], req.args[2], req.args[3],
					&ffa_msg, FFA_OK);

	expect_ffa_msg_send_direct_resp(
		req.source_id, req.destination_id,
		ROUTING_EXT_RC_BIT | ROUTING_EXT_ERR_BIT,
		SP_RESULT_FFA(FFA_BUSY), 0, 0, 0, &ffa_msg, result);

	expect_ffa_msg_wait(&ffa_msg, result);

	LONGS_EQUAL(SP_RESULT_FFA(result), sp_msg_send_rc_req(&req, &resp));
	MEMCMP_EQUAL(&empty_sp_msg, &resp, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_send_rc_req_rc_error)
{
	const uint16_t root_id = 1;
	const uint16_t own_id = 2;
	sp_result sp_err = SP_RESULT_NOT_FOUND;

	wait_and_receive_request(root_id, own_id);

	fill_sp_msg(&req);
	req.source_id = own_id;
	req.destination_id = root_id;

	fill_ffa_msg(&ffa_msg);
	ffa_msg.source_id = root_id;
	ffa_msg.destination_id = own_id;
	ffa_msg.args[0] = ROUTING_EXT_RC_BIT | ROUTING_EXT_ERR_BIT;
	ffa_msg.args[1] = sp_err;

	expect_ffa_msg_send_direct_resp(req.source_id, req.destination_id,
					ROUTING_EXT_RC_BIT, req.args[0],
					req.args[1], req.args[2], req.args[3],
					&ffa_msg, FFA_OK);

	LONGS_EQUAL(sp_err, sp_msg_send_rc_req(&req, &resp));
	MEMCMP_EQUAL(&empty_sp_msg, &resp, sizeof(empty_sp_msg));
}

TEST(sp_messaging, sp_msg_send_rc_req_success)
{
	const uint16_t root_id = 1;
	const uint16_t own_id = 2;

	wait_and_receive_request(root_id, own_id);

	fill_sp_msg(&req);
	req.source_id = own_id;
	req.destination_id = root_id;

	fill_ffa_msg(&ffa_msg);
	ffa_msg.source_id = root_id;
	ffa_msg.destination_id = own_id;
	ffa_msg.args[0] = ROUTING_EXT_RC_BIT;

	expect_ffa_msg_send_direct_resp(req.source_id, req.destination_id,
					ROUTING_EXT_RC_BIT, req.args[0],
					req.args[1], req.args[2], req.args[3],
					&ffa_msg, FFA_OK);

	LONGS_EQUAL(SP_RESULT_OK, sp_msg_send_rc_req(&req, &resp));
	ffa_and_sp_msg_equal(&ffa_msg, &resp);
}
#endif
