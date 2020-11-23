/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <service/crypto/client/cpp/crypto_client.h>
#include <service_locator.h>
#include <CppUTest/TestHarness.h>

/*
 * Service-level tests that focus on exercising each supported operation.
 * These are mainly valid behaviour tests with the goal of checking
 * that the number of operations supported is as expected.
 */
TEST_GROUP(CryptoServiceOpTests)
{
    void setup()
    {
        struct rpc_caller *caller;
        int status;

        m_rpc_session_handle = NULL;
        m_crypto_service_context = NULL;
        m_crypto_client = NULL;

        service_locator_init();

        m_crypto_service_context = service_locator_query("sn:trustedfirmware.org:crypto:0", &status);
        assert(m_crypto_service_context);

        m_rpc_session_handle = service_context_open(m_crypto_service_context, &caller);
        assert(m_rpc_session_handle);

        m_crypto_client = new crypto_client(caller);
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

TEST(CryptoServiceOpTests, generateVolatileKeys)
{
    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);
    psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_CURVE_SECP_R1));
    psa_set_key_bits(&attributes, 256);

    /* Generate first key */
     psa_key_handle_t key_handle_1;
    status = m_crypto_client->generate_key(&attributes, &key_handle_1);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* And another */
     psa_key_handle_t key_handle_2;
    status = m_crypto_client->generate_key(&attributes, &key_handle_2);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Expect the key handles to be different */
    CHECK(key_handle_1 != key_handle_2);

    /* Remove the keys */
    status = m_crypto_client->destroy_key(key_handle_1);
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->destroy_key(key_handle_2);
    CHECK_EQUAL(PSA_SUCCESS, status);

    psa_reset_key_attributes(&attributes);
}

TEST(CryptoServiceOpTests, generatePersistentKeys)
{
    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);
    psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_CURVE_SECP_R1));
    psa_set_key_bits(&attributes, 256);

    /* First try and generate a key with an invalid keu id */
    psa_key_id_t key_id_invalid = 0;
    psa_set_key_id(&attributes, key_id_invalid);
    psa_key_handle_t key_handle_invalid;
    status = m_crypto_client->generate_key(&attributes, &key_handle_invalid);
    CHECK_EQUAL(PSA_ERROR_INVALID_ARGUMENT, status);

    /* Generate first key */
    psa_key_id_t key_id_1 = 100000;
    psa_set_key_id(&attributes, key_id_1);
    psa_key_handle_t key_handle_1;
    status = m_crypto_client->generate_key(&attributes, &key_handle_1);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* And another */
    psa_key_id_t key_id_2 = 2;
    psa_set_key_id(&attributes, key_id_2);
    psa_key_handle_t key_handle_2;
    status = m_crypto_client->generate_key(&attributes, &key_handle_2);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Expect the key handles to be different */
    CHECK(key_handle_1 != key_handle_2);

    /* Obtain more handles using key_open */
    psa_key_handle_t key_handle_3;
    status = m_crypto_client->open_key(key_id_1, &key_handle_3);
    CHECK_EQUAL(PSA_SUCCESS, status);

    psa_key_handle_t key_handle_4;
    status = m_crypto_client->open_key(key_id_1, &key_handle_4);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Relinquish handles */
    status = m_crypto_client->close_key(key_handle_3);
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->close_key(key_handle_4);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Expect close handle to now be invalid */
    status = m_crypto_client->close_key(key_handle_4);
    CHECK_EQUAL(PSA_ERROR_INVALID_HANDLE, status);

    /* Remove the keys */
    status = m_crypto_client->destroy_key(key_handle_1);
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->destroy_key(key_handle_2);
    CHECK_EQUAL(PSA_SUCCESS, status);

    psa_reset_key_attributes(&attributes);
}

TEST(CryptoServiceOpTests, exportPublicKey)
{
    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_handle_t key_handle;

    psa_set_key_id(&attributes, 10);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);
    psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_CURVE_SECP_R1));
    psa_set_key_bits(&attributes, 256);

    /* Generate a key */
    status = m_crypto_client->generate_key(&attributes, &key_handle);
    CHECK_EQUAL(PSA_SUCCESS, status);

    psa_reset_key_attributes(&attributes);

    /* Export the public key */
    uint8_t key_buf[PSA_KEY_EXPORT_ECC_PUBLIC_KEY_MAX_SIZE(256)];
    size_t key_len = 0;

    status = m_crypto_client->export_public_key(key_handle, key_buf, sizeof(key_buf), &key_len);
    CHECK_EQUAL(PSA_SUCCESS, status);
    CHECK(key_len > 0);

    /* Remove the key */
    status = m_crypto_client->destroy_key(key_handle);
    CHECK_EQUAL(PSA_SUCCESS, status);
}

TEST(CryptoServiceOpTests, exportAndImportKeyPair)
{
    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_handle_t key_handle_1;
    psa_key_handle_t key_handle_2;

    psa_set_key_id(&attributes, 11);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_EXPORT);
    psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_CURVE_SECP_R1));
    psa_set_key_bits(&attributes, 256);

    /* Generate a key */
    status = m_crypto_client->generate_key(&attributes, &key_handle_1);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Export the key pair */
    uint8_t key_buf[PSA_KEY_EXPORT_ECC_KEY_PAIR_MAX_SIZE(256)];
    size_t key_len = 0;

    status = m_crypto_client->export_key(key_handle_1, key_buf, sizeof(key_buf), &key_len);
    CHECK_EQUAL(PSA_SUCCESS, status);
    CHECK(key_len > 0);

    /* Import the key pair value with a different key id */
    psa_set_key_id(&attributes, 12);
    status = m_crypto_client->import_key(&attributes, key_buf, key_len, &key_handle_2);
    CHECK_EQUAL(PSA_SUCCESS, status);

    psa_reset_key_attributes(&attributes);

    /* Remove the keys */
    status = m_crypto_client->destroy_key(key_handle_1);
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->destroy_key(key_handle_2);
    CHECK_EQUAL(PSA_SUCCESS, status);
}

TEST(CryptoServiceOpTests, signAndVerifyHash)
{
    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_handle_t key_handle;

    psa_set_key_id(&attributes, 13);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH);
    psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_CURVE_SECP_R1));
    psa_set_key_bits(&attributes, 256);

    /* Generate a key */
    status = m_crypto_client->generate_key(&attributes, &key_handle);
    CHECK_EQUAL(PSA_SUCCESS, status);

    psa_reset_key_attributes(&attributes);

    /* Sign a hash */
    uint8_t hash[20];
    uint8_t signature[PSA_SIGNATURE_MAX_SIZE];
    size_t signature_length;

    memset(hash, 0x71, sizeof(hash));

    status = m_crypto_client->sign_hash(key_handle,
        PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), hash, sizeof(hash),
        signature, sizeof(signature), &signature_length);

    CHECK_EQUAL(PSA_SUCCESS, status);
    CHECK(signature_length > 0);

    /* Verify the signature */
    status = m_crypto_client->verify_hash(key_handle,
        PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), hash, sizeof(hash),
        signature, signature_length);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Change the hash and expect verify to fail */
    hash[0] = 0x72;
    status = m_crypto_client->verify_hash(key_handle,
        PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), hash, sizeof(hash),
        signature, signature_length);
    CHECK_EQUAL(PSA_ERROR_INVALID_SIGNATURE, status);

    /* Remove the key */
    status = m_crypto_client->destroy_key(key_handle);
    CHECK_EQUAL(PSA_SUCCESS, status);
}

TEST(CryptoServiceOpTests, asymEncryptDecrypt)
{
    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_handle_t key_handle;

    psa_set_key_id(&attributes, 14);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&attributes, PSA_ALG_RSA_PKCS1V15_CRYPT);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_RSA_KEY_PAIR);
    psa_set_key_bits(&attributes, 256);

    /* Generate a key */
    status = m_crypto_client->generate_key(&attributes, &key_handle);
    CHECK_EQUAL(PSA_SUCCESS, status);

    psa_reset_key_attributes(&attributes);

    /* Encrypt a message */
    uint8_t message[] = {'q','u','i','c','k','b','r','o','w','n','f','o','x'};
    uint8_t ciphertext[256];
    size_t ciphertext_len = 0;

    status = m_crypto_client->asymmetric_encrypt(key_handle, PSA_ALG_RSA_PKCS1V15_CRYPT,
                            message, sizeof(message), NULL, 0,
                            ciphertext, sizeof(ciphertext), &ciphertext_len);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Decrypt it */
    uint8_t plaintext[256];
    size_t plaintext_len = 0;

    status = m_crypto_client->asymmetric_decrypt(key_handle, PSA_ALG_RSA_PKCS1V15_CRYPT,
                            ciphertext, ciphertext_len, NULL, 0,
                            plaintext, sizeof(plaintext), &plaintext_len);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Expect the encrypted/decrypted message to match theh original */
    CHECK_EQUAL(sizeof(message), plaintext_len);
    CHECK(memcmp(message, plaintext, plaintext_len) == 0);

    /* Remove the key */
    status = m_crypto_client->destroy_key(key_handle);
    CHECK_EQUAL(PSA_SUCCESS, status);
}

TEST(CryptoServiceOpTests, generateRandomNumbers)
{
    psa_status_t status;
    uint8_t num1_8bit[1];
    uint8_t num2_8bit[1];
    uint8_t num3_16bit[2];
    uint8_t num4_16bit[2];
    uint8_t num5_24bit[3];
    uint8_t num6_24bit[3];
    uint8_t num7_32bit[4];
    uint8_t num8_32bit[4];
    uint8_t num9_64bit[8];
    uint8_t num10_64bit[8];
    uint8_t num11_128bit[16];
    uint8_t num12_128bit[16];

    /* Clear all buffers */
    memset(num1_8bit, 0, sizeof(num1_8bit));
    memset(num2_8bit, 0, sizeof(num2_8bit));
    memset(num3_16bit, 0, sizeof(num3_16bit));
    memset(num4_16bit, 0, sizeof(num4_16bit));
    memset(num5_24bit, 0, sizeof(num5_24bit));
    memset(num6_24bit, 0, sizeof(num6_24bit));
    memset(num7_32bit, 0, sizeof(num7_32bit));
    memset(num8_32bit, 0, sizeof(num8_32bit));
    memset(num9_64bit, 0, sizeof(num9_64bit));
    memset(num10_64bit, 0, sizeof(num10_64bit));
    memset(num11_128bit, 0, sizeof(num11_128bit));
    memset(num12_128bit, 0, sizeof(num12_128bit));

    /* Generate some different size random numbers */
    status = m_crypto_client->generate_random(num1_8bit, sizeof(num1_8bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->generate_random(num2_8bit, sizeof(num2_8bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->generate_random(num3_16bit, sizeof(num3_16bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->generate_random(num4_16bit, sizeof(num4_16bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->generate_random(num5_24bit, sizeof(num5_24bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->generate_random(num6_24bit, sizeof(num6_24bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->generate_random(num7_32bit, sizeof(num7_32bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->generate_random(num8_32bit, sizeof(num8_32bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->generate_random(num9_64bit, sizeof(num9_64bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->generate_random(num10_64bit, sizeof(num10_64bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->generate_random(num11_128bit, sizeof(num11_128bit));
    CHECK_EQUAL(PSA_SUCCESS, status);
    status = m_crypto_client->generate_random(num12_128bit, sizeof(num12_128bit));
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Expect different numbers to be generated */
    CHECK(memcmp(num1_8bit, num2_8bit, sizeof(num1_8bit)) != 0);
    CHECK(memcmp(num3_16bit, num4_16bit, sizeof(num3_16bit)) != 0);
    CHECK(memcmp(num5_24bit, num6_24bit, sizeof(num5_24bit)) != 0);
    CHECK(memcmp(num7_32bit, num8_32bit, sizeof(num7_32bit)) != 0);
    CHECK(memcmp(num9_64bit, num10_64bit, sizeof(num9_64bit)) != 0);
    CHECK(memcmp(num11_128bit, num12_128bit, sizeof(num11_128bit)) != 0);
}
