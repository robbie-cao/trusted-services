/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <app/ts-demo/ts-demo.h>
#include <service/crypto/client/cpp/packed-c/packedc_crypto_client.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include <CppUTest/TestHarness.h>
#include <service_locator.h>
#include <service/crypto/client/cpp/crypto_client.h>


TEST_GROUP(TsDemoTests) {

    void setup()
    {
        struct rpc_caller *caller;
        int status;

        m_rpc_session_handle = NULL;
        m_crypto_service_context = NULL;
        m_crypto_client = NULL;

        service_locator_init();

        m_crypto_service_context = service_locator_query("sn:trustedfirmware.org:crypto:0", &status);
        CHECK(m_crypto_service_context);

        m_rpc_session_handle = service_context_open(m_crypto_service_context, TS_RPC_ENCODING_PACKED_C, &caller);
        CHECK(m_rpc_session_handle);

        m_crypto_client = new packedc_crypto_client(caller);
    }

    void teardown()
    {
        delete m_crypto_client;
        m_crypto_client = NULL;

        service_context_close(m_crypto_service_context, m_rpc_session_handle);
        m_rpc_session_handle = NULL;

        service_context_relinquish(m_crypto_service_context);
        m_crypto_service_context = NULL;
    }

    rpc_session_handle m_rpc_session_handle;
    struct service_context *m_crypto_service_context;
    crypto_client *m_crypto_client;
};

TEST(TsDemoTests, runTsDemo)
{
    int status = run_ts_demo(m_crypto_client, false);
    CHECK_EQUAL(0, status);
}
