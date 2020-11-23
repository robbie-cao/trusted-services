/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
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
        memset(&m_direct_caller, sizeof(m_direct_caller), 0);
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
    struct service_provider ep;

    service_provider_init(&ep, &ep, NULL, 0);
    struct rpc_caller *caller = direct_caller_init_default(&m_direct_caller,
                                            service_provider_get_call_ep(&ep));

    rpc_call_handle handle;
    uint8_t *req_buf;
    uint8_t *resp_buf;
    size_t req_len = 100;
    size_t resp_len;
    int opstatus;

    handle = rpc_caller_begin(caller, &req_buf, req_len);
    CHECK(handle);

    rpc_status_t rpc_status = rpc_caller_invoke(caller, handle, SOME_ARBITRARY_OPCODE,
                                    &opstatus, &resp_buf, &resp_len);

    rpc_caller_end(caller, handle);

    CHECK_EQUAL(TS_RPC_ERROR_INVALID_OPCODE, rpc_status);
}

TEST(ServiceFrameworkTests, serviceWithOps)
{
    /* Constructs a service endpoint with a couple of handlers */
    struct service_handler handlers[2];
    handlers[0].opcode = SOME_ARBITRARY_OPCODE;
    handlers[0].invoke = handlerThatSucceeds;
    handlers[1].opcode = ANOTHER_ARBITRARY_OPCODE;
    handlers[1].invoke = handlerThatFails;

    struct service_provider ep;

    service_provider_init(&ep, &ep, handlers, 2);
    struct rpc_caller *caller = direct_caller_init_default(&m_direct_caller,
                                            service_provider_get_call_ep(&ep));

    rpc_call_handle handle;
    rpc_status_t rpc_status;
    uint8_t *req_buf;
    uint8_t *resp_buf;
    size_t req_len = 100;
    size_t resp_len;
    int opstatus;
    std::string respString;

    /* Expect this call transaction to succeed */
    handle = rpc_caller_begin(caller, &req_buf, req_len);
    CHECK(handle);

    rpc_status = rpc_caller_invoke(caller, handle, SOME_ARBITRARY_OPCODE,
                                    &opstatus, &resp_buf, &resp_len);

    respString = std::string((const char*)resp_buf, resp_len);

    rpc_caller_end(caller, handle);

    CHECK_EQUAL(TS_RPC_CALL_ACCEPTED, rpc_status);
    CHECK_EQUAL(SERVICE_SPECIFIC_SUCCESS_CODE, opstatus);
    CHECK(respString == "Yay!");

    /* Expect this call transaction to fail */
    handle = rpc_caller_begin(caller, &req_buf, req_len);
    CHECK(handle);

    rpc_status = rpc_caller_invoke(caller, handle, ANOTHER_ARBITRARY_OPCODE,
        &opstatus, &resp_buf, &resp_len);

    respString = std::string((const char*)resp_buf, resp_len);

    rpc_caller_end(caller, handle);

    CHECK_EQUAL(TS_RPC_CALL_ACCEPTED, rpc_status);
    CHECK_EQUAL(SERVICE_SPECIFIC_ERROR_CODE, opstatus);
    CHECK(respString == "Ehh!");

    /* Try an unsupported opcode */
    handle = rpc_caller_begin(caller, &req_buf, req_len);
    CHECK(handle);

    rpc_status = rpc_caller_invoke(caller, handle, YET_ANOTHER_ARBITRARY_OPCODE,
        &opstatus, &resp_buf, &resp_len);

    rpc_caller_end(caller, handle);

    CHECK_EQUAL(TS_RPC_ERROR_INVALID_OPCODE, rpc_status);
}
