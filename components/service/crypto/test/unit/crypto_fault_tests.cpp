/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string>
#include <cstring>
#include <cstdint>
#include <service/crypto/client/test/standalone/standalone_crypto_client.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(CryptoFaultTests)
{
    void setup()
    {
        m_crypto_client = new standalone_crypto_client;
    }

    void teardown()
    {
        m_crypto_client->deinit();
        delete m_crypto_client;
        m_crypto_client = NULL;
    }

    test_crypto_client *m_crypto_client;
};

TEST(CryptoFaultTests, volatileKeyWithBrokenStorage)
{
    /* Inject broken secure storage fault */
    m_crypto_client->inject_fault(test_crypto_client::FAILED_TO_DISCOVER_SECURE_STORAGE);
    m_crypto_client->init();

    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);
    psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, 256);

    /* Expect generation of volatile key to still work with fault */
    psa_key_id_t key_id;
    status = m_crypto_client->generate_key(&attributes, &key_id);
    CHECK_EQUAL(PSA_SUCCESS, status);
    CHECK(0 != key_id);

    psa_reset_key_attributes(&attributes);
}

TEST(CryptoFaultTests, persistentKeysWithBrokenStorage)
{
    /* Inject broken secure storage fault */
    m_crypto_client->inject_fault(test_crypto_client::FAILED_TO_DISCOVER_SECURE_STORAGE);
    m_crypto_client->init();

    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);
    psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, 256);

    /* Expect persist key generation to fail */
    psa_key_id_t key_id;
    psa_set_key_id(&attributes, 1);
    status = m_crypto_client->generate_key(&attributes, &key_id);
    CHECK(PSA_SUCCESS != status);

    psa_reset_key_attributes(&attributes);
}

TEST(CryptoFaultTests, randomNumbersWithBrokenStorage)
{
    /* Inject broken secure storage fault */
    m_crypto_client->inject_fault(test_crypto_client::FAILED_TO_DISCOVER_SECURE_STORAGE);
    m_crypto_client->init();

    psa_status_t status;
    uint8_t num12_128bit[16];

    memset(num12_128bit, 0, sizeof(num12_128bit));

    /* Expect random number generation to work, despite the broken storage */
    status = m_crypto_client->generate_random(num12_128bit, sizeof(num12_128bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
}
