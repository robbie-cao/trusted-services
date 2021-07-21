/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string>
#include <cstring>
#include <cstdint>
#include <vector>
#include <CppUTest/TestHarness.h>
#include "crypto_service_scenarios.h"
#include "crypto_test_vectors.h"

crypto_service_scenarios::crypto_service_scenarios(crypto_client *crypto_client) :
	m_crypto_client(crypto_client)
{

}

crypto_service_scenarios::~crypto_service_scenarios()
{
	delete m_crypto_client;
	m_crypto_client = NULL;
}

void crypto_service_scenarios::generateVolatileKeys()
{
	psa_status_t status;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);
	psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_bits(&attributes, 256);

	/* Generate first key */
	 psa_key_id_t key_id_1;
	status = m_crypto_client->generate_key(&attributes, &key_id_1);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* And another */
	 psa_key_id_t key_id_2;
	status = m_crypto_client->generate_key(&attributes, &key_id_2);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Expect the key IDs to be different */
	CHECK(key_id_1 != key_id_2);

	/* Remove the keys */
	status = m_crypto_client->destroy_key(key_id_1);
	CHECK_EQUAL(PSA_SUCCESS, status);
	status = m_crypto_client->destroy_key(key_id_2);
	CHECK_EQUAL(PSA_SUCCESS, status);

	psa_reset_key_attributes(&attributes);
}

void crypto_service_scenarios::generatePersistentKeys()
{
	psa_status_t status;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);
	psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_bits(&attributes, 256);

	/* First try and generate a key with an invalid key id */
	psa_key_id_t key_id;
	psa_set_key_id(&attributes, 0);
	status = m_crypto_client->generate_key(&attributes, &key_id);
	CHECK_EQUAL(PSA_ERROR_INVALID_HANDLE, status);

	/* Generate first key */
	psa_key_id_t key_id_1;
	psa_set_key_id(&attributes, 100000);
	status = m_crypto_client->generate_key(&attributes, &key_id_1);
	CHECK_EQUAL(PSA_SUCCESS, status);
	CHECK_EQUAL(100000, key_id_1);

	/* And another */
	psa_key_id_t key_id_2;
	psa_set_key_id(&attributes, 2);
	status = m_crypto_client->generate_key(&attributes, &key_id_2);
	CHECK_EQUAL(PSA_SUCCESS, status);
	CHECK_EQUAL(2, key_id_2);

	/* Remove the keys */
	status = m_crypto_client->destroy_key(key_id_1);
	CHECK_EQUAL(PSA_SUCCESS, status);
	status = m_crypto_client->destroy_key(key_id_2);
	CHECK_EQUAL(PSA_SUCCESS, status);

	psa_reset_key_attributes(&attributes);
}

void crypto_service_scenarios::exportPublicKey()
{
	psa_status_t status;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_key_id_t key_id;

	psa_set_key_id(&attributes, 10);
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);
	psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_bits(&attributes, 256);

	/* Generate a key */
	status = m_crypto_client->generate_key(&attributes, &key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);

	psa_reset_key_attributes(&attributes);

	/* Export the public key */
	uint8_t key_buf[PSA_EXPORT_PUBLIC_KEY_MAX_SIZE];
	size_t key_len = 0;

	status = m_crypto_client->export_public_key(key_id, key_buf, sizeof(key_buf), &key_len);
	CHECK_EQUAL(PSA_SUCCESS, status);
	CHECK_TRUE(key_len > 0);

	/* Remove the key */
	status = m_crypto_client->destroy_key(key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);
}

void crypto_service_scenarios::exportAndImportKeyPair()
{
	psa_status_t status;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_key_id_t key_id_1;
	psa_key_id_t key_id_2;

	psa_set_key_id(&attributes, 11);
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_EXPORT);
	psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_bits(&attributes, 256);

	/* Generate a key */
	status = m_crypto_client->generate_key(&attributes, &key_id_1);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Export the key pair */
	uint8_t key_buf[PSA_EXPORT_PUBLIC_KEY_MAX_SIZE];
	size_t key_len = 0;

	status = m_crypto_client->export_key(key_id_1, key_buf, sizeof(key_buf), &key_len);
	CHECK_EQUAL(PSA_SUCCESS, status);
	CHECK(key_len > 0);

	/* Import the key pair value with a different key id */
	psa_set_key_id(&attributes, 12);
	status = m_crypto_client->import_key(&attributes, key_buf, key_len, &key_id_2);
	CHECK_EQUAL(PSA_SUCCESS, status);

	psa_reset_key_attributes(&attributes);

	/* Remove the keys */
	status = m_crypto_client->destroy_key(key_id_1);
	CHECK_EQUAL(PSA_SUCCESS, status);
	status = m_crypto_client->destroy_key(key_id_2);
	CHECK_EQUAL(PSA_SUCCESS, status);
}

void crypto_service_scenarios::signAndVerifyHash()
{
	psa_status_t status;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_key_id_t key_id;

	psa_set_key_id(&attributes, 13);
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH);
	psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_bits(&attributes, 256);

	/* Generate a key */
	status = m_crypto_client->generate_key(&attributes, &key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);

	psa_reset_key_attributes(&attributes);

	/* Sign a hash */
	uint8_t hash[20];
	uint8_t signature[PSA_SIGNATURE_MAX_SIZE];
	size_t signature_length;

	memset(hash, 0x71, sizeof(hash));

	status = m_crypto_client->sign_hash(key_id,
		PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), hash, sizeof(hash),
		signature, sizeof(signature), &signature_length);

	CHECK_EQUAL(PSA_SUCCESS, status);
	CHECK(signature_length > 0);

	/* Verify the signature */
	status = m_crypto_client->verify_hash(key_id,
		PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), hash, sizeof(hash),
		signature, signature_length);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Change the hash and expect verify to fail */
	hash[0] = 0x72;
	status = m_crypto_client->verify_hash(key_id,
		PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), hash, sizeof(hash),
		signature, signature_length);
	CHECK_EQUAL(PSA_ERROR_INVALID_SIGNATURE, status);

	/* Remove the key */
	status = m_crypto_client->destroy_key(key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);
}

void crypto_service_scenarios::signAndVerifyEat()
{
	/* Sign and verify a hash using EAT key type and algorithm */
	psa_status_t status;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_key_id_t key_id;

	psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH);

	psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_bits(&attributes, 256);

	/* Generate a key */
	status = m_crypto_client->generate_key(&attributes, &key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);

	psa_reset_key_attributes(&attributes);

	/* Sign a hash */
	uint8_t hash[64];
	uint8_t signature[PSA_SIGNATURE_MAX_SIZE];
	size_t signature_length;

	memset(hash, 0x71, sizeof(hash));

	status = m_crypto_client->sign_hash(key_id,
		PSA_ALG_ECDSA(PSA_ALG_SHA_256), hash, sizeof(hash),
		signature, sizeof(signature), &signature_length);

	CHECK_EQUAL(PSA_SUCCESS, status);
	CHECK(signature_length > 0);

	/* Verify the signature */
	status = m_crypto_client->verify_hash(key_id,
		PSA_ALG_ECDSA(PSA_ALG_SHA_256), hash, sizeof(hash),
		signature, signature_length);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Change the hash and expect verify to fail */
	hash[0] = 0x72;
	status = m_crypto_client->verify_hash(key_id,
		PSA_ALG_ECDSA(PSA_ALG_SHA_256), hash, sizeof(hash),
		signature, signature_length);
	CHECK_EQUAL(PSA_ERROR_INVALID_SIGNATURE, status);

	/* Remove the key */
	status = m_crypto_client->destroy_key(key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);
}

void crypto_service_scenarios::asymEncryptDecrypt()
{
	psa_status_t status;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_key_id_t key_id;

	psa_set_key_id(&attributes, 14);
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
	psa_set_key_algorithm(&attributes, PSA_ALG_RSA_PKCS1V15_CRYPT);
	psa_set_key_type(&attributes, PSA_KEY_TYPE_RSA_KEY_PAIR);
	psa_set_key_bits(&attributes, 256);

	/* Generate a key */
	status = m_crypto_client->generate_key(&attributes, &key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);

	psa_reset_key_attributes(&attributes);

	/* Encrypt a message */
	uint8_t message[] = {'q','u','i','c','k','b','r','o','w','n','f','o','x'};
	uint8_t ciphertext[256];
	size_t ciphertext_len = 0;

	status = m_crypto_client->asymmetric_encrypt(key_id, PSA_ALG_RSA_PKCS1V15_CRYPT,
							message, sizeof(message), NULL, 0,
							ciphertext, sizeof(ciphertext), &ciphertext_len);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Decrypt it */
	uint8_t plaintext[256];
	size_t plaintext_len = 0;

	status = m_crypto_client->asymmetric_decrypt(key_id, PSA_ALG_RSA_PKCS1V15_CRYPT,
							ciphertext, ciphertext_len, NULL, 0,
							plaintext, sizeof(plaintext), &plaintext_len);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Expect the encrypted/decrypted message to match theh original */
	CHECK_EQUAL(sizeof(message), plaintext_len);
	MEMCMP_EQUAL(message, plaintext, plaintext_len);

	/* Remove the key */
	status = m_crypto_client->destroy_key(key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);
}

void crypto_service_scenarios::asymEncryptDecryptWithSalt()
{
	psa_status_t status;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_key_id_t key_id;

	psa_set_key_id(&attributes, 15);
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
	psa_set_key_algorithm(&attributes,  PSA_ALG_RSA_OAEP(PSA_ALG_SHA_256));
	psa_set_key_type(&attributes, PSA_KEY_TYPE_RSA_KEY_PAIR);
	psa_set_key_bits(&attributes, 1024);

	/* Generate a key */
	status = m_crypto_client->generate_key(&attributes, &key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);

	psa_reset_key_attributes(&attributes);

	/* Encrypt a message */
	uint8_t message[] = {'q','u','i','c','k','b','r','o','w','n','f','o','x'};
	uint8_t ciphertext[128];
	size_t ciphertext_len = 0;

	/* With salt */
	uint8_t salt[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

	status = m_crypto_client->asymmetric_encrypt(key_id, PSA_ALG_RSA_OAEP(PSA_ALG_SHA_256),
							message, sizeof(message),
							salt, sizeof(salt),
							ciphertext, sizeof(ciphertext), &ciphertext_len);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Decrypt it */
	uint8_t plaintext[256];
	size_t plaintext_len = 0;

	status = m_crypto_client->asymmetric_decrypt(key_id, PSA_ALG_RSA_OAEP(PSA_ALG_SHA_256),
							ciphertext, ciphertext_len,
							salt, sizeof(salt),
							plaintext, sizeof(plaintext), &plaintext_len);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Expect the encrypted/decrypted message to match theh original */
	CHECK_EQUAL(sizeof(message), plaintext_len);
	MEMCMP_EQUAL(message, plaintext, plaintext_len);

	/* Remove the key */
	status = m_crypto_client->destroy_key(key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);
}

void crypto_service_scenarios::calculateHash()
{
	psa_status_t status;
	std::vector<uint8_t> input;
	std::vector<uint8_t> expected_output;
	uint8_t output[PSA_HASH_MAX_SIZE];
	size_t output_len;

	crypto_test_vectors::plaintext_1_len_610(input);
	crypto_test_vectors::sha256_1(expected_output);

	uint32_t op_handle = 0;

	status = m_crypto_client->hash_setup(&op_handle, PSA_ALG_SHA_256);
	CHECK_EQUAL(PSA_SUCCESS, status);

	status = m_crypto_client->hash_update(op_handle, &input[0], input.size());
	CHECK_EQUAL(PSA_SUCCESS, status);

	status = m_crypto_client->hash_finish(op_handle, output, sizeof(output), &output_len);
	CHECK_EQUAL(PSA_SUCCESS, status);

	UNSIGNED_LONGS_EQUAL(expected_output.size(), output_len);
	MEMCMP_EQUAL(&expected_output[0], &output[0], output_len);
}

void crypto_service_scenarios::generateRandomNumbers()
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

	/* For larger numbers, it should be improbable that numbers are the same  */
	CHECK(memcmp(num5_24bit, num6_24bit, sizeof(num5_24bit)) != 0);
	CHECK(memcmp(num7_32bit, num8_32bit, sizeof(num7_32bit)) != 0);
	CHECK(memcmp(num9_64bit, num10_64bit, sizeof(num9_64bit)) != 0);
	CHECK(memcmp(num11_128bit, num12_128bit, sizeof(num11_128bit)) != 0);
}
