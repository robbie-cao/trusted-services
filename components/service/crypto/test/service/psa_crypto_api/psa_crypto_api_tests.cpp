/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "psa_crypto_api_client.h"
#include <psa/crypto.h>
#include <service/crypto/client/psa/psa_crypto_client.h>
#include <service/crypto/test/service/crypto_service_scenarios.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include <service_locator.h>
#include <CppUTest/TestHarness.h>

/*
 * Service-level tests that use the psa crypto api
 */
TEST_GROUP(PsaCryptoApiTests)
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

        m_rpc_session_handle = service_context_open(m_crypto_service_context, TS_RPC_ENCODING_PACKED_C, &caller);
        CHECK_TRUE(m_rpc_session_handle);

        psa_crypto_client_init(caller);
        psa_crypto_init();

        m_scenarios = new crypto_service_scenarios(new psa_crypto_api_client());
    }

    void teardown()
    {
        delete m_scenarios;
        m_scenarios = NULL;

        psa_crypto_client_deinit();

        service_context_close(m_crypto_service_context, m_rpc_session_handle);
        m_rpc_session_handle = NULL;

        service_context_relinquish(m_crypto_service_context);
        m_crypto_service_context = NULL;
    }

    rpc_session_handle m_rpc_session_handle;
    struct service_context *m_crypto_service_context;
    crypto_service_scenarios *m_scenarios;
};

TEST(PsaCryptoApiTests, generateVolatileKeys)
{
    m_scenarios->generateVolatileKeys();
}

TEST(PsaCryptoApiTests, generatePersistentKeys)
{
    m_scenarios->generatePersistentKeys();
}

TEST(PsaCryptoApiTests, exportPublicKey)
{
    m_scenarios->exportPublicKey();
}

TEST(PsaCryptoApiTests, exportAndImportKeyPair)
{
    m_scenarios->exportAndImportKeyPair();
}

TEST(PsaCryptoApiTests, signAndVerifyHash)
{
    m_scenarios->signAndVerifyHash();
}

TEST(PsaCryptoApiTests, asymEncryptDecrypt)
{
    m_scenarios->asymEncryptDecrypt();
}

TEST(PsaCryptoApiTests, asymEncryptDecryptWithSalt)
{
    m_scenarios->asymEncryptDecryptWithSalt();
}

TEST(PsaCryptoApiTests, generateRandomNumbers)
{
    m_scenarios->generateRandomNumbers();
}
