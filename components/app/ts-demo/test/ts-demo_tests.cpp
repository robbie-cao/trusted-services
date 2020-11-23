/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <app/ts-demo/ts-demo.h>
#include <service/crypto/client/test/test_crypto_client.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(TsDemoTests) {

    void setup()
    {
        m_crypto_client = test_crypto_client::create_default();
        m_crypto_client->init();
    }

    void teardown()
    {
        m_crypto_client->deinit();
        delete m_crypto_client;
        m_crypto_client = NULL;
    }

    test_crypto_client *m_crypto_client;
};

TEST(TsDemoTests, runTsDemo)
{
    int status = run_ts_demo(m_crypto_client, false);
    CHECK_EQUAL(0, status);
}
