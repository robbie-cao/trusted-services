/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string>
#include <cstring>
#include <service/common/provider/service_provider.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <rpc/direct/direct_caller.h>
#include <CppUTest/TestHarness.h>


TEST_GROUP(ServiceFrameworkTests)
{
	static rpc_status_t handlerThatSucceeds(void *context, struct call_req* req)
	{
		(void)context;

		struct call_param_buf *respBuf = call_req_get_resp_buf(req);

		std::string responseString("Yay!");
		respBuf->data_len = responseString.copy((char*)respBuf->data, respBuf->size);

		call_req_set_opstatus(req, SERVICE_SPECIFIC_SUCCESS_CODE);

		return TS_RPC_CALL_ACCEPTED;
	}

	static rpc_status_t handlerThatFails(void *context, struct call_req* req)
	{
		(void)context;

		struct call_param_buf *respBuf = call_req_get_resp_buf(req);

		std::string responseString("Ehh!");
		respBuf->data_len = responseString.copy((char*)respBuf->data, respBuf->size);

		call_req_set_opstatus(req, SERVICE_SPECIFIC_ERROR_CODE);

		return TS_RPC_CALL_ACCEPTED;
	}

	void setup()
	{
		memset(&m_direct_caller, 0, sizeof(m_direct_caller));
	}

	void teardown()
	{
		direct_caller_deinit(&m_direct_caller);
	}

	static const uint32_t SOME_ARBITRARY_OPCODE = 666;
	static const uint32_t ANOTHER_ARBITRARY_OPCODE = 901;
	static const uint32_t YET_ANOTHER_ARBITRARY_OPCODE = 7;
	static const int SERVICE_SPECIFIC_ERROR_CODE = 101;
	static const int SERVICE_SPECIFIC_SUCCESS_CODE = 100;

	struct direct_caller m_direct_caller;
};

TEST(ServiceFrameworkTests, serviceWithNoOps)
{
	/* Constructs a service endpoint with no handlers */
	struct service_provider service_provider;

	service_provider_init(&service_provider, &service_provider, NULL, 0);
	struct rpc_caller *caller = direct_caller_init_default(&m_direct_caller,
											service_provider_get_rpc_interface(&service_provider));

	rpc_call_handle handle;
	uint8_t *req_buf;
	uint8_t *resp_buf;
	size_t req_len = 100;
	size_t resp_len;
	rpc_opstatus_t opstatus;

	handle = rpc_caller_begin(caller, &req_buf, req_len);
	CHECK_TRUE(handle);

	rpc_status_t rpc_status = rpc_caller_invoke(caller, handle, SOME_ARBITRARY_OPCODE,
									&opstatus, &resp_buf, &resp_len);

	rpc_caller_end(caller, handle);

	LONGS_EQUAL(TS_RPC_ERROR_INVALID_OPCODE, rpc_status);
}

TEST(ServiceFrameworkTests, serviceWithOps)
{
	/* Constructs a service endpoint with a couple of handlers */
	struct service_handler handlers[2];
	handlers[0].opcode = SOME_ARBITRARY_OPCODE;
	handlers[0].invoke = handlerThatSucceeds;
	handlers[1].opcode = ANOTHER_ARBITRARY_OPCODE;
	handlers[1].invoke = handlerThatFails;

	struct service_provider service_provider;

	service_provider_init(&service_provider, &service_provider, handlers, 2);
	struct rpc_caller *caller = direct_caller_init_default(&m_direct_caller,
											service_provider_get_rpc_interface(&service_provider));

	rpc_call_handle handle;
	rpc_status_t rpc_status;
	uint8_t *req_buf;
	uint8_t *resp_buf;
	size_t req_len = 100;
	size_t resp_len;
	rpc_opstatus_t opstatus;
	std::string respString;

	/* Expect this call transaction to succeed */
	handle = rpc_caller_begin(caller, &req_buf, req_len);
	CHECK_TRUE(handle);

	rpc_status = rpc_caller_invoke(caller, handle, SOME_ARBITRARY_OPCODE,
									&opstatus, &resp_buf, &resp_len);

	respString = std::string((const char*)resp_buf, resp_len);

	rpc_caller_end(caller, handle);

	LONGS_EQUAL(TS_RPC_CALL_ACCEPTED, rpc_status);
	LONGS_EQUAL(SERVICE_SPECIFIC_SUCCESS_CODE, opstatus);
	STRCMP_EQUAL("Yay!", respString.c_str());

	/* Expect this call transaction to fail */
	handle = rpc_caller_begin(caller, &req_buf, req_len);
	CHECK_TRUE(handle);

	rpc_status = rpc_caller_invoke(caller, handle, ANOTHER_ARBITRARY_OPCODE,
		&opstatus, &resp_buf, &resp_len);

	respString = std::string((const char*)resp_buf, resp_len);

	rpc_caller_end(caller, handle);

	LONGS_EQUAL(TS_RPC_CALL_ACCEPTED, rpc_status);
	LONGS_EQUAL(SERVICE_SPECIFIC_ERROR_CODE, opstatus);
	STRCMP_EQUAL("Ehh!", respString.c_str());

	/* Try an unsupported opcode */
	handle = rpc_caller_begin(caller, &req_buf, req_len);
	CHECK_TRUE(handle);

	rpc_status = rpc_caller_invoke(caller, handle, YET_ANOTHER_ARBITRARY_OPCODE,
		&opstatus, &resp_buf, &resp_len);

	rpc_caller_end(caller, handle);

	LONGS_EQUAL(TS_RPC_ERROR_INVALID_OPCODE, rpc_status);
}

TEST(ServiceFrameworkTests, serviceProviderChain)
{
	/* Construct the base service provider */
	struct service_handler base_handlers[1];
	base_handlers[0].opcode = 100;
	base_handlers[0].invoke = handlerThatSucceeds;

	struct service_provider base_provider;
	service_provider_init(&base_provider, &base_provider, base_handlers, 1);

	/* Construct a sub provider and extend the base */
	struct service_handler sub0_handlers[1];
	sub0_handlers[0].opcode = 200;
	sub0_handlers[0].invoke = handlerThatSucceeds;

	struct service_provider sub0_provider;
	service_provider_init(&sub0_provider, &sub0_provider, sub0_handlers, 1);
	service_provider_extend(&base_provider, &sub0_provider);

	/* Construct another sub provider and extend the base */
	struct service_handler sub1_handlers[1];
	sub1_handlers[0].opcode = 300;
	sub1_handlers[0].invoke = handlerThatSucceeds;

	struct service_provider sub1_provider;
	service_provider_init(&sub1_provider, &sub1_provider, sub1_handlers, 1);
	service_provider_extend(&base_provider, &sub1_provider);

	/* Use a direct_caller to make RPC calls to the base provider at the head of the chain */
	struct rpc_caller *caller = direct_caller_init_default(&m_direct_caller,
									service_provider_get_rpc_interface(&base_provider));

	rpc_call_handle handle;
	rpc_status_t rpc_status;
	uint8_t *req_buf;
	uint8_t *resp_buf;
	size_t req_len = 100;
	size_t resp_len;
	rpc_opstatus_t opstatus;
	std::string respString;

	/* Expect calls that will be handled by all three chained service providers to succeed */
	handle = rpc_caller_begin(caller, &req_buf, req_len);
	CHECK_TRUE(handle);

	rpc_status = rpc_caller_invoke(caller, handle, 100,
									&opstatus, &resp_buf, &resp_len);

	respString = std::string((const char*)resp_buf, resp_len);

	rpc_caller_end(caller, handle);

	LONGS_EQUAL(TS_RPC_CALL_ACCEPTED, rpc_status);
	LONGS_EQUAL(SERVICE_SPECIFIC_SUCCESS_CODE, opstatus);
	STRCMP_EQUAL("Yay!", respString.c_str());

	/* This one should beb handled by sub0 */
	handle = rpc_caller_begin(caller, &req_buf, req_len);
	CHECK_TRUE(handle);

	rpc_status = rpc_caller_invoke(caller, handle, 200,
									&opstatus, &resp_buf, &resp_len);

	respString = std::string((const char*)resp_buf, resp_len);

	rpc_caller_end(caller, handle);

	LONGS_EQUAL(TS_RPC_CALL_ACCEPTED, rpc_status);
	LONGS_EQUAL(SERVICE_SPECIFIC_SUCCESS_CODE, opstatus);
	STRCMP_EQUAL("Yay!", respString.c_str());

	/* This one should beb handled by sub1 */
	handle = rpc_caller_begin(caller, &req_buf, req_len);
	CHECK_TRUE(handle);

	rpc_status = rpc_caller_invoke(caller, handle, 300,
									&opstatus, &resp_buf, &resp_len);

	respString = std::string((const char*)resp_buf, resp_len);

	rpc_caller_end(caller, handle);

	LONGS_EQUAL(TS_RPC_CALL_ACCEPTED, rpc_status);
	LONGS_EQUAL(SERVICE_SPECIFIC_SUCCESS_CODE, opstatus);
	STRCMP_EQUAL("Yay!", respString.c_str());

	/* Try an unsupported opcode */
	handle = rpc_caller_begin(caller, &req_buf, req_len);
	CHECK_TRUE(handle);

	rpc_status = rpc_caller_invoke(caller, handle, 400,
		&opstatus, &resp_buf, &resp_len);

	rpc_caller_end(caller, handle);

	LONGS_EQUAL(TS_RPC_ERROR_INVALID_OPCODE, rpc_status);
}
