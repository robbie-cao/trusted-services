/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string>
#include <cstring>
#include <cstdint>
#include <vector>
#include <CppUTest/TestHarness.h>
#include "crypto_service_scenarios.h"

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
	CHECK_EQUAL(PSA_ERROR_INVALID_ARGUMENT, status);

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

void crypto_service_scenarios::copyKey()
{
	psa_status_t status;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_COPY);
	psa_set_key_algorithm(&attributes, PSA_ALG_RSA_PKCS1V15_CRYPT);
	psa_set_key_type(&attributes, PSA_KEY_TYPE_RSA_KEY_PAIR);
	psa_set_key_bits(&attributes, 256);

	/* Generate a key */
	psa_key_id_t key_id_1;
	status = m_crypto_client->generate_key(&attributes, &key_id_1);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Copy it */
	psa_key_id_t key_id_2;
	status = m_crypto_client->copy_key(key_id_1, &attributes, &key_id_2);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Expect the copied key attributes to match the original */
	psa_key_attributes_t copy_attributes = PSA_KEY_ATTRIBUTES_INIT;

	status = m_crypto_client->get_key_attributes(key_id_2, &copy_attributes);
	CHECK_EQUAL(PSA_SUCCESS, status);

	UNSIGNED_LONGS_EQUAL(
		psa_get_key_type(&attributes), psa_get_key_type(&copy_attributes));
	UNSIGNED_LONGS_EQUAL(
		psa_get_key_algorithm(&attributes), psa_get_key_algorithm(&copy_attributes));
	UNSIGNED_LONGS_EQUAL(
		psa_get_key_bits(&attributes), psa_get_key_bits(&copy_attributes));
	UNSIGNED_LONGS_EQUAL(
		psa_get_key_usage_flags(&attributes), psa_get_key_usage_flags(&copy_attributes));
	UNSIGNED_LONGS_EQUAL(
		psa_get_key_lifetime(&attributes), psa_get_key_lifetime(&copy_attributes));

	/* Remove the keys */
	status = m_crypto_client->destroy_key(key_id_1);
	CHECK_EQUAL(PSA_SUCCESS, status);
	status = m_crypto_client->destroy_key(key_id_2);
	CHECK_EQUAL(PSA_SUCCESS, status);

	psa_reset_key_attributes(&attributes);
	psa_reset_key_attributes(&copy_attributes);
}

void crypto_service_scenarios::purgeKey()
{
	psa_status_t status;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	/* Generate a persistent key */
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);
	psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_bits(&attributes, 256);
	psa_set_key_id(&attributes, 100002);

	psa_key_id_t key_id;
	status = m_crypto_client->generate_key(&attributes, &key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Perform purge */
	status = m_crypto_client->purge_key(key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Expect key to still exist when destroyed */
	status = m_crypto_client->destroy_key(key_id);
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

void crypto_service_scenarios::signAndVerifyMessage()
{
	psa_status_t status;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_key_id_t key_id;

	psa_set_key_id(&attributes, 14);
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
	psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_bits(&attributes, 256);

	/* Generate a key */
	status = m_crypto_client->generate_key(&attributes, &key_id);
	CHECK_EQUAL(PSA_SUCCESS, status);

	psa_reset_key_attributes(&attributes);

	/* Sign a message */
	uint8_t message[21];
	uint8_t signature[PSA_SIGNATURE_MAX_SIZE];
	size_t signature_length;

	memset(message, 0x99, sizeof(message));

	status = m_crypto_client->sign_message(key_id,
		PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), message, sizeof(message),
		signature, sizeof(signature), &signature_length);

	CHECK_EQUAL(PSA_SUCCESS, status);
	CHECK(signature_length > 0);

	/* Verify the signature */
	status = m_crypto_client->verify_message(key_id,
		PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), message, sizeof(message),
		signature, signature_length);
	CHECK_EQUAL(PSA_SUCCESS, status);

	/* Change the message and expect verify to fail */
	message[0] = 0x72;
	status = m_crypto_client->verify_message(key_id,
		PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), message, sizeof(message),
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

	psa_set_key_id(&attributes, 15);
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

	psa_set_key_id(&attributes, 16);
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

void crypto_service_scenarios::verifypkcs7signature(void)
{
	psa_status_t status;

	unsigned char hash[] = { 0x56, 0xcf, 0x42, 0x1c, 0x35, 0x78, 0x2f, 0x6f, 0x77, 0xdd, 0x2b,
				 0x44, 0x0b, 0xb4, 0xdf, 0x34, 0x56, 0x6b, 0x69, 0xc4, 0x51, 0x9b,
				 0x47, 0x7b, 0x64, 0xfd, 0x56, 0x56, 0x25, 0xd6, 0x47, 0x27 };

	unsigned char public_key[] = {
		0x30, 0x82, 0x03, 0x07, 0x30, 0x82, 0x01, 0xef, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02,
		0x14, 0x5a, 0x8e, 0x8a, 0x75, 0x09, 0xe8, 0x96, 0xed, 0x26, 0x29, 0xca, 0xd6, 0x01,
		0xfc, 0xce, 0xb4, 0x70, 0x70, 0x68, 0x55, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48,
		0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30, 0x13, 0x31, 0x11, 0x30, 0x0f,
		0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x08, 0x54, 0x65, 0x73, 0x74, 0x20, 0x50, 0x4b,
		0x31, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x33, 0x30, 0x38, 0x32, 0x32, 0x30, 0x38, 0x31,
		0x30, 0x33, 0x37, 0x5a, 0x17, 0x0d, 0x33, 0x33, 0x30, 0x38, 0x31, 0x39, 0x30, 0x38,
		0x31, 0x30, 0x33, 0x37, 0x5a, 0x30, 0x13, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55,
		0x04, 0x03, 0x0c, 0x08, 0x54, 0x65, 0x73, 0x74, 0x20, 0x50, 0x4b, 0x31, 0x30, 0x82,
		0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
		0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02, 0x82,
		0x01, 0x01, 0x00, 0xd5, 0x55, 0x6e, 0x9f, 0xa8, 0x92, 0x68, 0x2b, 0x3c, 0xbd, 0xbc,
		0x37, 0xd5, 0x2f, 0x5e, 0xf1, 0x70, 0x76, 0x7b, 0x5e, 0x54, 0xd5, 0x89, 0x90, 0x5a,
		0xeb, 0x01, 0x63, 0x6c, 0x34, 0xe9, 0x54, 0xa0, 0x06, 0x31, 0xf0, 0xff, 0x9b, 0xd8,
		0x80, 0x2a, 0x3d, 0x42, 0x37, 0xab, 0x37, 0xd9, 0x22, 0xff, 0x66, 0xd1, 0x02, 0xb9,
		0xbc, 0xe2, 0x8a, 0x45, 0xc8, 0xfe, 0x6f, 0x6c, 0xfc, 0xca, 0x5e, 0x90, 0x5c, 0xb1,
		0xc6, 0xd8, 0x2f, 0x59, 0xac, 0x46, 0x36, 0x0c, 0x7d, 0x39, 0xc4, 0x5f, 0xd4, 0xae,
		0x1f, 0x81, 0x6e, 0x79, 0xdf, 0xe5, 0xfd, 0x8f, 0xbb, 0x28, 0xc8, 0x7d, 0x0e, 0x46,
		0x66, 0xb9, 0x4b, 0x30, 0xf6, 0x9b, 0xc2, 0xff, 0x0d, 0xc7, 0x93, 0x5d, 0xd8, 0xbb,
		0x00, 0x7b, 0x35, 0x6a, 0x79, 0xa8, 0x47, 0xd6, 0xf5, 0x54, 0xc6, 0x28, 0x88, 0x58,
		0x7d, 0x34, 0xdc, 0x41, 0x29, 0x9c, 0xef, 0x54, 0x00, 0x2c, 0xce, 0xcd, 0xad, 0x07,
		0x38, 0x98, 0x07, 0x05, 0xf9, 0x4a, 0x67, 0x1e, 0xeb, 0x14, 0x33, 0x73, 0x6e, 0x3b,
		0xa4, 0x4d, 0xc5, 0x0b, 0x6a, 0xfb, 0x76, 0xa5, 0xef, 0x4f, 0xb4, 0x59, 0xf0, 0x2a,
		0xce, 0x8c, 0xdc, 0xdf, 0xd1, 0x3d, 0x52, 0x36, 0xb9, 0x05, 0xc4, 0x11, 0x7f, 0xe8,
		0x5a, 0x7f, 0xfc, 0xfc, 0xdc, 0x53, 0x62, 0x34, 0x69, 0xa7, 0x67, 0x49, 0x74, 0xb9,
		0xd1, 0x40, 0x72, 0x7a, 0x06, 0x8b, 0xb6, 0xc5, 0x0e, 0x4c, 0x99, 0xf8, 0xcb, 0x3a,
		0x96, 0xe9, 0x8e, 0x49, 0x87, 0xe8, 0xa6, 0x80, 0x43, 0x0e, 0x66, 0x87, 0x9a, 0xca,
		0xd6, 0x58, 0x97, 0x5d, 0xd4, 0x9f, 0x2f, 0x00, 0x9b, 0xed, 0x94, 0x8f, 0x13, 0xc2,
		0xd8, 0xd1, 0x35, 0x24, 0x96, 0x61, 0x66, 0xed, 0xa7, 0xd1, 0x36, 0xb4, 0xe9, 0x28,
		0x9c, 0x36, 0xf9, 0x2d, 0x38, 0x03, 0xaf, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x53,
		0x30, 0x51, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0xc1,
		0xb8, 0xa7, 0xa0, 0xb9, 0x1c, 0x8c, 0x02, 0x5a, 0x17, 0x3e, 0x68, 0x94, 0xc0, 0x88,
		0xcb, 0x4e, 0x63, 0x7f, 0x2d, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18,
		0x30, 0x16, 0x80, 0x14, 0xc1, 0xb8, 0xa7, 0xa0, 0xb9, 0x1c, 0x8c, 0x02, 0x5a, 0x17,
		0x3e, 0x68, 0x94, 0xc0, 0x88, 0xcb, 0x4e, 0x63, 0x7f, 0x2d, 0x30, 0x0f, 0x06, 0x03,
		0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30,
		0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00,
		0x03, 0x82, 0x01, 0x01, 0x00, 0x49, 0x3a, 0x18, 0xc9, 0x09, 0x77, 0xfa, 0xde, 0xbe,
		0xd4, 0x1c, 0xdb, 0xbd, 0x42, 0x53, 0x25, 0x21, 0x45, 0xe3, 0xcc, 0xe8, 0xa4, 0xe5,
		0x68, 0xf6, 0xba, 0x09, 0x01, 0xad, 0x9e, 0x75, 0x9f, 0x1e, 0x5c, 0x07, 0xef, 0xcd,
		0x0b, 0x4a, 0x26, 0x5b, 0x03, 0x52, 0x04, 0xb5, 0x27, 0x5c, 0x18, 0x1e, 0x58, 0x54,
		0xa3, 0xc8, 0xbd, 0x87, 0xc3, 0xa1, 0x7d, 0x8a, 0x9b, 0x3e, 0xa8, 0xbf, 0x76, 0xa8,
		0x3c, 0xaa, 0x54, 0xfa, 0x78, 0x30, 0xfc, 0xa8, 0x52, 0xca, 0x20, 0x8d, 0x72, 0x29,
		0x61, 0x38, 0x10, 0xcb, 0x36, 0x50, 0x3f, 0xf3, 0x8c, 0xc6, 0xb5, 0xd6, 0xa3, 0xf0,
		0x6f, 0x76, 0x30, 0xb7, 0xbd, 0x2b, 0x5d, 0x2d, 0x10, 0x63, 0x17, 0xbd, 0x0f, 0x54,
		0x88, 0xb6, 0x78, 0x6e, 0x06, 0x8d, 0x65, 0x0e, 0x26, 0xea, 0x4e, 0x3c, 0xb4, 0xf0,
		0x74, 0x0b, 0xd6, 0xef, 0x5a, 0x04, 0x77, 0x66, 0xc8, 0x74, 0x5e, 0xe1, 0xd7, 0x37,
		0xcc, 0x74, 0x5f, 0x32, 0xb1, 0x42, 0x70, 0x5f, 0x05, 0xfa, 0x9f, 0x0d, 0xb6, 0xf7,
		0xd9, 0xf7, 0x42, 0xbe, 0x2b, 0xf4, 0x5f, 0xf1, 0x65, 0x2c, 0xaf, 0xde, 0xfb, 0xf4,
		0x69, 0xa4, 0x45, 0x1f, 0xa0, 0x39, 0x37, 0xda, 0x81, 0x07, 0xd2, 0x3e, 0xd9, 0x5b,
		0xc4, 0xb2, 0x7c, 0xea, 0x17, 0xaf, 0x05, 0x68, 0x70, 0xfd, 0x85, 0x81, 0x15, 0x16,
		0xa8, 0xc3, 0xbf, 0xbf, 0x00, 0xbf, 0x17, 0xef, 0x78, 0xc9, 0x40, 0xd1, 0x2a, 0x11,
		0x00, 0xcc, 0x39, 0x40, 0xae, 0x79, 0x30, 0xa8, 0x27, 0xb6, 0x6c, 0x64, 0x26, 0xcb,
		0x20, 0xdb, 0xad, 0x75, 0x75, 0xe8, 0xa0, 0x50, 0x84, 0x2b, 0x00, 0x93, 0xdf, 0xf8,
		0x79, 0x69, 0xef, 0x6d, 0x1c, 0xdf, 0xc6, 0x40, 0x39, 0xc9, 0x8e, 0xda, 0x70, 0xcf,
		0x1c, 0x4d, 0x78, 0xc6, 0x9c, 0xa7, 0xe0, 0x8e, 0x25
	};

	unsigned char signature[] = {
		0x30, 0x82, 0x04, 0x9d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07,
		0x02, 0xa0, 0x82, 0x04, 0x8e, 0x30, 0x82, 0x04, 0x8a, 0x02, 0x01, 0x01, 0x31, 0x0f,
		0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05,
		0x00, 0x30, 0x0b, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01,
		0xa0, 0x82, 0x03, 0x0b, 0x30, 0x82, 0x03, 0x07, 0x30, 0x82, 0x01, 0xef, 0xa0, 0x03,
		0x02, 0x01, 0x02, 0x02, 0x14, 0x5a, 0x8e, 0x8a, 0x75, 0x09, 0xe8, 0x96, 0xed, 0x26,
		0x29, 0xca, 0xd6, 0x01, 0xfc, 0xce, 0xb4, 0x70, 0x70, 0x68, 0x55, 0x30, 0x0d, 0x06,
		0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30, 0x13,
		0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x08, 0x54, 0x65, 0x73,
		0x74, 0x20, 0x50, 0x4b, 0x31, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x33, 0x30, 0x38, 0x32,
		0x32, 0x30, 0x38, 0x31, 0x30, 0x33, 0x37, 0x5a, 0x17, 0x0d, 0x33, 0x33, 0x30, 0x38,
		0x31, 0x39, 0x30, 0x38, 0x31, 0x30, 0x33, 0x37, 0x5a, 0x30, 0x13, 0x31, 0x11, 0x30,
		0x0f, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x08, 0x54, 0x65, 0x73, 0x74, 0x20, 0x50,
		0x4b, 0x31, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
		0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82,
		0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xd5, 0x55, 0x6e, 0x9f, 0xa8, 0x92, 0x68,
		0x2b, 0x3c, 0xbd, 0xbc, 0x37, 0xd5, 0x2f, 0x5e, 0xf1, 0x70, 0x76, 0x7b, 0x5e, 0x54,
		0xd5, 0x89, 0x90, 0x5a, 0xeb, 0x01, 0x63, 0x6c, 0x34, 0xe9, 0x54, 0xa0, 0x06, 0x31,
		0xf0, 0xff, 0x9b, 0xd8, 0x80, 0x2a, 0x3d, 0x42, 0x37, 0xab, 0x37, 0xd9, 0x22, 0xff,
		0x66, 0xd1, 0x02, 0xb9, 0xbc, 0xe2, 0x8a, 0x45, 0xc8, 0xfe, 0x6f, 0x6c, 0xfc, 0xca,
		0x5e, 0x90, 0x5c, 0xb1, 0xc6, 0xd8, 0x2f, 0x59, 0xac, 0x46, 0x36, 0x0c, 0x7d, 0x39,
		0xc4, 0x5f, 0xd4, 0xae, 0x1f, 0x81, 0x6e, 0x79, 0xdf, 0xe5, 0xfd, 0x8f, 0xbb, 0x28,
		0xc8, 0x7d, 0x0e, 0x46, 0x66, 0xb9, 0x4b, 0x30, 0xf6, 0x9b, 0xc2, 0xff, 0x0d, 0xc7,
		0x93, 0x5d, 0xd8, 0xbb, 0x00, 0x7b, 0x35, 0x6a, 0x79, 0xa8, 0x47, 0xd6, 0xf5, 0x54,
		0xc6, 0x28, 0x88, 0x58, 0x7d, 0x34, 0xdc, 0x41, 0x29, 0x9c, 0xef, 0x54, 0x00, 0x2c,
		0xce, 0xcd, 0xad, 0x07, 0x38, 0x98, 0x07, 0x05, 0xf9, 0x4a, 0x67, 0x1e, 0xeb, 0x14,
		0x33, 0x73, 0x6e, 0x3b, 0xa4, 0x4d, 0xc5, 0x0b, 0x6a, 0xfb, 0x76, 0xa5, 0xef, 0x4f,
		0xb4, 0x59, 0xf0, 0x2a, 0xce, 0x8c, 0xdc, 0xdf, 0xd1, 0x3d, 0x52, 0x36, 0xb9, 0x05,
		0xc4, 0x11, 0x7f, 0xe8, 0x5a, 0x7f, 0xfc, 0xfc, 0xdc, 0x53, 0x62, 0x34, 0x69, 0xa7,
		0x67, 0x49, 0x74, 0xb9, 0xd1, 0x40, 0x72, 0x7a, 0x06, 0x8b, 0xb6, 0xc5, 0x0e, 0x4c,
		0x99, 0xf8, 0xcb, 0x3a, 0x96, 0xe9, 0x8e, 0x49, 0x87, 0xe8, 0xa6, 0x80, 0x43, 0x0e,
		0x66, 0x87, 0x9a, 0xca, 0xd6, 0x58, 0x97, 0x5d, 0xd4, 0x9f, 0x2f, 0x00, 0x9b, 0xed,
		0x94, 0x8f, 0x13, 0xc2, 0xd8, 0xd1, 0x35, 0x24, 0x96, 0x61, 0x66, 0xed, 0xa7, 0xd1,
		0x36, 0xb4, 0xe9, 0x28, 0x9c, 0x36, 0xf9, 0x2d, 0x38, 0x03, 0xaf, 0x02, 0x03, 0x01,
		0x00, 0x01, 0xa3, 0x53, 0x30, 0x51, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04,
		0x16, 0x04, 0x14, 0xc1, 0xb8, 0xa7, 0xa0, 0xb9, 0x1c, 0x8c, 0x02, 0x5a, 0x17, 0x3e,
		0x68, 0x94, 0xc0, 0x88, 0xcb, 0x4e, 0x63, 0x7f, 0x2d, 0x30, 0x1f, 0x06, 0x03, 0x55,
		0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xc1, 0xb8, 0xa7, 0xa0, 0xb9, 0x1c,
		0x8c, 0x02, 0x5a, 0x17, 0x3e, 0x68, 0x94, 0xc0, 0x88, 0xcb, 0x4e, 0x63, 0x7f, 0x2d,
		0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05, 0x30, 0x03,
		0x01, 0x01, 0xff, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
		0x01, 0x0b, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x49, 0x3a, 0x18, 0xc9, 0x09,
		0x77, 0xfa, 0xde, 0xbe, 0xd4, 0x1c, 0xdb, 0xbd, 0x42, 0x53, 0x25, 0x21, 0x45, 0xe3,
		0xcc, 0xe8, 0xa4, 0xe5, 0x68, 0xf6, 0xba, 0x09, 0x01, 0xad, 0x9e, 0x75, 0x9f, 0x1e,
		0x5c, 0x07, 0xef, 0xcd, 0x0b, 0x4a, 0x26, 0x5b, 0x03, 0x52, 0x04, 0xb5, 0x27, 0x5c,
		0x18, 0x1e, 0x58, 0x54, 0xa3, 0xc8, 0xbd, 0x87, 0xc3, 0xa1, 0x7d, 0x8a, 0x9b, 0x3e,
		0xa8, 0xbf, 0x76, 0xa8, 0x3c, 0xaa, 0x54, 0xfa, 0x78, 0x30, 0xfc, 0xa8, 0x52, 0xca,
		0x20, 0x8d, 0x72, 0x29, 0x61, 0x38, 0x10, 0xcb, 0x36, 0x50, 0x3f, 0xf3, 0x8c, 0xc6,
		0xb5, 0xd6, 0xa3, 0xf0, 0x6f, 0x76, 0x30, 0xb7, 0xbd, 0x2b, 0x5d, 0x2d, 0x10, 0x63,
		0x17, 0xbd, 0x0f, 0x54, 0x88, 0xb6, 0x78, 0x6e, 0x06, 0x8d, 0x65, 0x0e, 0x26, 0xea,
		0x4e, 0x3c, 0xb4, 0xf0, 0x74, 0x0b, 0xd6, 0xef, 0x5a, 0x04, 0x77, 0x66, 0xc8, 0x74,
		0x5e, 0xe1, 0xd7, 0x37, 0xcc, 0x74, 0x5f, 0x32, 0xb1, 0x42, 0x70, 0x5f, 0x05, 0xfa,
		0x9f, 0x0d, 0xb6, 0xf7, 0xd9, 0xf7, 0x42, 0xbe, 0x2b, 0xf4, 0x5f, 0xf1, 0x65, 0x2c,
		0xaf, 0xde, 0xfb, 0xf4, 0x69, 0xa4, 0x45, 0x1f, 0xa0, 0x39, 0x37, 0xda, 0x81, 0x07,
		0xd2, 0x3e, 0xd9, 0x5b, 0xc4, 0xb2, 0x7c, 0xea, 0x17, 0xaf, 0x05, 0x68, 0x70, 0xfd,
		0x85, 0x81, 0x15, 0x16, 0xa8, 0xc3, 0xbf, 0xbf, 0x00, 0xbf, 0x17, 0xef, 0x78, 0xc9,
		0x40, 0xd1, 0x2a, 0x11, 0x00, 0xcc, 0x39, 0x40, 0xae, 0x79, 0x30, 0xa8, 0x27, 0xb6,
		0x6c, 0x64, 0x26, 0xcb, 0x20, 0xdb, 0xad, 0x75, 0x75, 0xe8, 0xa0, 0x50, 0x84, 0x2b,
		0x00, 0x93, 0xdf, 0xf8, 0x79, 0x69, 0xef, 0x6d, 0x1c, 0xdf, 0xc6, 0x40, 0x39, 0xc9,
		0x8e, 0xda, 0x70, 0xcf, 0x1c, 0x4d, 0x78, 0xc6, 0x9c, 0xa7, 0xe0, 0x8e, 0x25, 0x31,
		0x82, 0x01, 0x56, 0x30, 0x82, 0x01, 0x52, 0x02, 0x01, 0x01, 0x30, 0x2b, 0x30, 0x13,
		0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x08, 0x54, 0x65, 0x73,
		0x74, 0x20, 0x50, 0x4b, 0x31, 0x02, 0x14, 0x5a, 0x8e, 0x8a, 0x75, 0x09, 0xe8, 0x96,
		0xed, 0x26, 0x29, 0xca, 0xd6, 0x01, 0xfc, 0xce, 0xb4, 0x70, 0x70, 0x68, 0x55, 0x30,
		0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00,
		0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05,
		0x00, 0x04, 0x82, 0x01, 0x00, 0x33, 0x93, 0x6d, 0x8d, 0xf6, 0xf9, 0x3f, 0xbe, 0x37,
		0xed, 0xc7, 0xb3, 0xf6, 0xd3, 0xd3, 0x55, 0x2e, 0x30, 0x98, 0xd9, 0x50, 0x7f, 0xc5,
		0x3c, 0x12, 0x9c, 0xcf, 0x53, 0x1e, 0x0d, 0xd7, 0x5c, 0x94, 0x67, 0x15, 0x07, 0x17,
		0x6d, 0x41, 0x93, 0x9d, 0x1a, 0x36, 0x92, 0x8c, 0x46, 0xb9, 0x3a, 0x4d, 0xed, 0xd6,
		0xe0, 0x23, 0x50, 0x7e, 0xfd, 0x4d, 0xd9, 0x59, 0xbc, 0xaf, 0x7b, 0x4f, 0x83, 0x99,
		0xd4, 0x11, 0xf5, 0xb4, 0x24, 0xba, 0x35, 0x87, 0x22, 0xbf, 0xa6, 0x3d, 0xe0, 0xcb,
		0x52, 0xbd, 0xcb, 0xb0, 0xae, 0x86, 0xb8, 0x97, 0x8e, 0xc7, 0xc5, 0x9a, 0x14, 0x15,
		0xc8, 0x71, 0x57, 0x40, 0x1c, 0x47, 0x11, 0x68, 0xfd, 0x27, 0xe4, 0xde, 0x2d, 0xbb,
		0x27, 0xea, 0xa3, 0xe9, 0x75, 0xe3, 0x74, 0x33, 0x87, 0x04, 0xe1, 0x72, 0x1a, 0x24,
		0x75, 0x94, 0xfd, 0x6a, 0x9c, 0xef, 0xc5, 0x4b, 0xee, 0x86, 0x51, 0x11, 0xb2, 0x59,
		0x07, 0x33, 0x5c, 0xa1, 0x11, 0xd0, 0x83, 0xd2, 0x99, 0x40, 0x92, 0xe8, 0xc8, 0xe5,
		0x20, 0xd9, 0x0e, 0x25, 0x2b, 0x27, 0x92, 0xc7, 0xbc, 0xe0, 0xb7, 0xd8, 0x16, 0x15,
		0x38, 0x25, 0xfc, 0x85, 0x75, 0xaf, 0x9c, 0x12, 0x87, 0x88, 0x2f, 0x6d, 0xd6, 0x24,
		0x89, 0xe5, 0x5e, 0x0c, 0x0d, 0x03, 0xf6, 0x5e, 0x03, 0xf9, 0xf7, 0x6b, 0x92, 0xeb,
		0xe4, 0x4b, 0xab, 0x58, 0x81, 0xe8, 0x73, 0x90, 0x0e, 0x84, 0x9e, 0x9d, 0x24, 0x25,
		0x91, 0x75, 0x8a, 0xdc, 0x92, 0xe6, 0x42, 0x52, 0x38, 0xcf, 0x63, 0xd9, 0xb7, 0xe6,
		0xcd, 0x80, 0x35, 0x8a, 0xe4, 0x1d, 0xb9, 0xe2, 0x86, 0xc2, 0xbf, 0x44, 0x83, 0x20,
		0x3a, 0x59, 0x9b, 0xbf, 0x64, 0xc0, 0xae, 0x2a, 0x6b, 0xe5, 0x06, 0x4a, 0x86, 0x26,
		0xb3, 0x48, 0xdb, 0xad, 0x31, 0x2f, 0x71, 0xef, 0x33,
	};

	/* Inject error into hash value and expect failure */
	hash[0] = ~hash[0];
	status = m_crypto_client->verify_pkcs7_signature((const uint8_t *)signature,
							 sizeof(signature), (const uint8_t *)hash,
							 sizeof(hash), (const uint8_t *)public_key,
							 sizeof(public_key));
	hash[0] = ~hash[0];

	/*
	 * If the functionality to be tested here is missing from mbedtls let's return.
	 * Expected: MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED
	 */
	if(status == (-0x0072))
		return;

	CHECK(status != 0);

	/* Inject error into the public key format and expect failure */
	public_key[0] = ~public_key[0];
	status = m_crypto_client->verify_pkcs7_signature((const uint8_t *)signature,
							 sizeof(signature), (const uint8_t *)hash,
							 sizeof(hash), (const uint8_t *)public_key,
							 sizeof(public_key));
	public_key[0] = ~public_key[0];

	CHECK(status != 0);

	/* Inject error into the public key value and expect failure
	 * The beginning of the key is found by using parser.
	 */
	int first_byte_of_public_key = 152;
	public_key[first_byte_of_public_key] = ~public_key[first_byte_of_public_key];
	status = m_crypto_client->verify_pkcs7_signature((const uint8_t *)signature,
							 sizeof(signature), (const uint8_t *)hash,
							 sizeof(hash), (const uint8_t *)public_key,
							 sizeof(public_key));
	public_key[first_byte_of_public_key] = ~public_key[first_byte_of_public_key];

	CHECK(status != 0);

	/* Inject error into the signature format and expect failure */
	signature[0] = ~signature[0];
	status = m_crypto_client->verify_pkcs7_signature((const uint8_t *)signature,
							 sizeof(signature), (const uint8_t *)hash,
							 sizeof(hash), (const uint8_t *)public_key,
							 sizeof(public_key));
	signature[0] = ~signature[0];

	CHECK(status != 0);

	/* Inject error into the signature value what is at the end of the vector */
	signature[sizeof(signature) - 1] = ~signature[sizeof(signature) - 1];
	status = m_crypto_client->verify_pkcs7_signature((const uint8_t *)signature,
							 sizeof(signature), (const uint8_t *)hash,
							 sizeof(hash), (const uint8_t *)public_key,
							 sizeof(public_key));
	signature[sizeof(signature) - 1] = ~signature[sizeof(signature) - 1];

	CHECK(status != 0);

	/* Verify correct signature */
	status = m_crypto_client->verify_pkcs7_signature((const uint8_t *)signature,
							 sizeof(signature), (const uint8_t *)hash,
							 sizeof(hash), (const uint8_t *)public_key,
							 sizeof(public_key));

	CHECK(status == 0);
}