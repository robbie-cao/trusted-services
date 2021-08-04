/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <service/crypto/client/cpp/protocol/protobuf/protobuf_crypto_client.h>
#include <service/crypto/test/service/crypto_service_scenarios.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include <service_locator.h>
#include <CppUTest/TestHarness.h>

/*
 * Service-level tests that use the Protobuf access protocol serialization
 */
TEST_GROUP(CryptoServiceProtobufTests)
{
    void setup()
    {
        struct rpc_caller *caller;
        int status;

        m_rpc_session_handle = NULL;
        m_crypto_service_context = NULL;
        m_scenarios = NULL;

        service_locator_init();

        m_crypto_service_context = service_locator_query("sn:trustedfirmware.org:crypto:0", &status);
        CHECK_TRUE(m_crypto_service_context);

        m_rpc_session_handle = service_context_open(m_crypto_service_context, TS_RPC_ENCODING_PROTOBUF, &caller);
        CHECK_TRUE(m_rpc_session_handle);

        m_scenarios = new crypto_service_scenarios(new protobuf_crypto_client(caller));
    }

    void teardown()
    {
        delete m_scenarios;
        m_scenarios = NULL;

        service_context_close(m_crypto_service_context, m_rpc_session_handle);
        m_rpc_session_handle = NULL;

        service_context_relinquish(m_crypto_service_context);
        m_crypto_service_context = NULL;
    }

    rpc_session_handle m_rpc_session_handle;
    struct service_context *m_crypto_service_context;
    crypto_service_scenarios *m_scenarios;
};

TEST(CryptoServiceProtobufTests, generateVolatileKeys)
{
    m_scenarios->generateVolatileKeys();
}

TEST(CryptoServiceProtobufTests, generatePersistentKeys)
{
    m_scenarios->generatePersistentKeys();
}

TEST(CryptoServiceProtobufTests, exportPublicKey)
{
    m_scenarios->exportPublicKey();
}

TEST(CryptoServiceProtobufTests, exportAndImportKeyPair)
{
    m_scenarios->exportAndImportKeyPair();
}

TEST(CryptoServiceProtobufTests, signAndVerifyHash)
{
    m_scenarios->signAndVerifyHash();
}

TEST(CryptoServiceProtobufTests, asymEncryptDecrypt)
{
    m_scenarios->asymEncryptDecrypt();
}

TEST(CryptoServiceProtobufTests, asymEncryptDecryptWithSalt)
{
    m_scenarios->asymEncryptDecryptWithSalt();
}

TEST(CryptoServiceProtobufTests, generateRandomNumbers)
{
    m_scenarios->generateRandomNumbers();
}
