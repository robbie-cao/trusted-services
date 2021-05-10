/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <cstdio>
#include <cstring>
#include <psa/crypto.h>
#include <CppUTest/TestHarness.h>

/* Tests to prototype each of the Crypto operations used for the PoC Crypto SP
 * demonstrator.
 */
TEST_GROUP(PocCryptoOpTests) {

    void setup() {
        print_info = false;
        (void)psa_crypto_init();
    }

    void generateEcdsaKeyPair() {
        psa_status_t status;
        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

        psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
        psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);
        psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
        psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
        psa_set_key_bits(&attributes, 256);

        status = psa_generate_key(&attributes, &ecdsa_key_id);
        CHECK_EQUAL(PSA_SUCCESS, status);

        psa_reset_key_attributes(&attributes);

        if (print_info)	{
            printf("Generated ECDSA key pair\n");
            printf("    Inputs: attributes: %ld\n", sizeof(attributes));
            printf("    Outputs: id: %ld status: %ld\n", sizeof(ecdsa_key_id), sizeof(status));
        }
    }

    void exportPublicKey() {
        psa_status_t status;
        exported_key_len = 0;
        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

        status = psa_get_key_attributes(ecdsa_key_id, &attributes);

        CHECK_EQUAL(PSA_SUCCESS, status);

        size_t max_export_size = PSA_EXPORT_PUBLIC_KEY_OUTPUT_SIZE(PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR(psa_get_key_type(&attributes)),
                                    psa_get_key_bits(&attributes));

        status = psa_export_public_key(ecdsa_key_id, exported_key, sizeof(exported_key),
                                        &exported_key_len);

        CHECK_EQUAL(PSA_SUCCESS, status);
        CHECK_EQUAL(max_export_size, exported_key_len);

        psa_reset_key_attributes(&attributes);

        if (print_info) {
            printf("Exported a public key\n");
            printf("    Inputs: id: %ld\n", sizeof(ecdsa_key_id));
            printf("    Outputs: exported_length: %ld status: %ld\n", sizeof(exported_key_len), sizeof(status));
            printf("    Outputs: space of key value: %ld bytes\n", exported_key_len);
        }
    }

    void signMessage() {
        psa_status_t status;
        uint8_t hash[20];
        uint8_t signature[PSA_SIGNATURE_MAX_SIZE];
        size_t signature_length;
        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

        memset(hash, 0x55, sizeof(hash));

        status = psa_get_key_attributes(ecdsa_key_id, &attributes);

        CHECK_EQUAL(PSA_SUCCESS, status);

        status = psa_sign_hash(ecdsa_key_id, psa_get_key_algorithm(&attributes),
                                hash, sizeof(hash),
                                signature, sizeof(signature),
                                &signature_length);
        CHECK_EQUAL(PSA_SUCCESS, status);

        psa_reset_key_attributes(&attributes);

        if (print_info) {
            printf("Signed a message\n");
            printf("    Inputs: id: %ld algo 1 message %ld\n", sizeof(ecdsa_key_id), sizeof(hash));
            printf("    Outputs: signature: %ld\n", signature_length);
        }
    }

    bool print_info;
    psa_key_id_t ecdsa_key_id;
    uint8_t exported_key[PSA_EXPORT_PUBLIC_KEY_MAX_SIZE];
    size_t exported_key_len;
};

TEST(PocCryptoOpTests, checkOpSequence) {

    generateEcdsaKeyPair();
    exportPublicKey();
    signMessage();
}