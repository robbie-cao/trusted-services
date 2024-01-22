/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <service/crypto/client/cpp/crypto_client.h>
#include "ts-demo.h"

class ts_demo {
public:
    ts_demo(crypto_client *crypto_client, bool is_verbose) :
        m_crypto_client(crypto_client),
        m_signing_key_id(0),
        m_encryption_key_id(0),
        m_verbose(is_verbose),
        m_all_ok(true) {

    }

    ~ts_demo() {

    }

    bool is_all_ok() const {
        return m_all_ok;
    }

    void print_intro() {

        if (m_verbose) {
            printf("\nDemonstrates use of trusted services from an application");
            printf("\n---------------------------------------------------------");
            printf("\nA client requests a set of crypto operations performed by");
            printf("\nthe Crypto service.  Key storage for persistent keys is");
            printf("\nprovided by the Secure Storage service via the ITS client.\n");
            printf("\n");
        }
    }

    void wait(int seconds) {

        if (m_verbose) sleep(seconds);
    }

    void print_status(psa_status_t status) {

            if (m_verbose) {

            if (status == PSA_SUCCESS) {
                printf("\n\tOperation successful\n");
            }
            else {
                printf("\n\tOperation failed. op error: %d RPC call status %d\n",
                    status, m_crypto_client->err_rpc_status());
            }
        }
    }

    void print_byte_array(const uint8_t *array, size_t len)
    {
        size_t count = 0;
        size_t column = 0;

        while (count < len) {

            if (column == 0) printf("\n\t\t");
            else printf(" ");

            printf("%02X", array[count]);

            ++count;
            column = (column +1) % 8;
        }

        printf("\n");
    }

    void generate_signing_key()
    {
        psa_status_t status;
        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

        psa_set_key_id(&attributes, SIGNING_KEY_ID);
        psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH);
        psa_set_key_algorithm(&attributes, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
        psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
        psa_set_key_bits(&attributes, 256);

        if (m_verbose) printf("Generating ECC signing key");

        status = m_crypto_client->generate_key(&attributes, &m_signing_key_id);
        psa_reset_key_attributes(&attributes);

        print_status(status);

        m_all_ok &= (status == PSA_SUCCESS);
    }

    void sign_and_verify_message(const char *message)
    {

        psa_status_t status;
        uint8_t hash[100];
        size_t hash_len = strlen(message) + 1;

        if (hash_len > sizeof(hash)) hash_len = sizeof(hash) - 1;

        memset(hash, 0, sizeof(hash));
        memcpy(hash, message, hash_len);

	    /* Sign message */
        uint8_t signature[PSA_SIGNATURE_MAX_SIZE];
        size_t signature_length;

        if (m_verbose) printf("Signing message: \"%s\" using key: %d", hash, m_signing_key_id);

        status = m_crypto_client->sign_hash(m_signing_key_id,
            PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), hash, hash_len,
            signature, sizeof(signature), &signature_length);

        print_status(status);

        if (m_verbose && (status == PSA_SUCCESS)) {
            printf("\tSignature bytes: ");
            print_byte_array(signature, signature_length);
        }

        m_all_ok &= (status == PSA_SUCCESS);

        /* Verify signature against original message */
        if (m_verbose) printf("Verify signature using original message: \"%s\"", hash);

        status = m_crypto_client->verify_hash(m_signing_key_id,
            PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), hash, hash_len,
            signature, signature_length);

        print_status(status);

        m_all_ok &= (status == PSA_SUCCESS);

        /* Verify signature against modified message */
        hash[0] = '!';
        if (m_verbose) printf("Verify signature using modified message: \"%s\"", hash);

        status = m_crypto_client->verify_hash(m_signing_key_id,
            PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), hash, hash_len,
            signature, signature_length);

        if (status == PSA_ERROR_INVALID_SIGNATURE) {
            if (m_verbose) printf("\n\tSuccessfully detected modified message\n");
        }
        else {
            print_status(status);
        }

        m_all_ok &= (status != PSA_SUCCESS);
    }

    void generate_asymmetric_encryption_key()
    {
        psa_status_t status;
        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

        psa_set_key_id(&attributes, ENCRYPTION_KEY_ID);
        psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
        psa_set_key_algorithm(&attributes, PSA_ALG_RSA_PKCS1V15_CRYPT);
        psa_set_key_type(&attributes, PSA_KEY_TYPE_RSA_KEY_PAIR);
        psa_set_key_bits(&attributes, 1024);

        if (m_verbose) printf("Generating RSA encryption key");

        status = m_crypto_client->generate_key(&attributes, &m_encryption_key_id);
        psa_reset_key_attributes(&attributes);

        print_status(status);

        m_all_ok &= (status == PSA_SUCCESS);
    }

    void encrypt_add_decrypt_message(const char *message)
    {
        psa_status_t status;
        size_t message_len = strlen(message) + 1;

        /* Encrypt a message */
        if (m_verbose) printf("Encrypting message: \"%s\" using RSA key: %d", message, m_encryption_key_id);

        uint8_t ciphertext[256];
        size_t ciphertext_len = 0;

        status = m_crypto_client->asymmetric_encrypt(m_encryption_key_id, PSA_ALG_RSA_PKCS1V15_CRYPT,
                                (const uint8_t*)message, message_len, NULL, 0,
                                ciphertext, sizeof(ciphertext), &ciphertext_len);
        print_status(status);

        if (m_verbose && (status == PSA_SUCCESS)) {
            printf("\tEncrypted message: ");
            print_byte_array(ciphertext, ciphertext_len);
        }

        m_all_ok &= (status == PSA_SUCCESS);

        /* Decrypt it */
        if (m_verbose) printf("Decrypting message using RSA key: %d", m_encryption_key_id);

        uint8_t plaintext[256];
        size_t plaintext_len = 0;

        status = m_crypto_client->asymmetric_decrypt(m_encryption_key_id, PSA_ALG_RSA_PKCS1V15_CRYPT,
                                ciphertext, ciphertext_len, NULL, 0,
                                plaintext, sizeof(plaintext), &plaintext_len);
        print_status(status);

        if (m_verbose && (status == PSA_SUCCESS)) {

            if ((plaintext_len == message_len) &&
                (memcmp(message, plaintext, plaintext_len) == 0)) {
                if (m_verbose) printf("\tDecrypted message: \"%s\"\n", plaintext);
            }
            else {
                printf("\tDecrypted message is different from original!: ");
                print_byte_array(plaintext, plaintext_len);
            }
        }

        m_all_ok &= (status == PSA_SUCCESS);
    }

    void export_public_key()
    {
        psa_status_t status;
        uint8_t key_buf[PSA_EXPORT_PUBLIC_KEY_MAX_SIZE];
        size_t key_len = 0;

        if (m_verbose) printf("Exporting public key: %d", m_signing_key_id);

        status = m_crypto_client->export_public_key(m_signing_key_id, key_buf, sizeof(key_buf), &key_len);

        print_status(status);

        if (m_verbose && (status == PSA_SUCCESS)) {
            printf("\tPublic key bytes: ");
            print_byte_array(key_buf, key_len);
        }

        m_all_ok &= (status == PSA_SUCCESS);
    }

    void generate_random_number(size_t length)
    {
        psa_status_t status;
        uint8_t buffer[length];

        if (m_verbose) printf("Generating random bytes length: %ld", length);

        status = m_crypto_client->generate_random(buffer, length);

        print_status(status);

        if (m_verbose && (status == PSA_SUCCESS)) {
            printf("\tRandom bytes: ");
            print_byte_array(buffer, length);
        }

        m_all_ok &= (status == PSA_SUCCESS);
    }

    void destroy_keys()
    {
        psa_status_t status;

        if (m_verbose) printf("Destroying signing key: %d", m_signing_key_id);
        status = m_crypto_client->destroy_key(m_signing_key_id);
        print_status(status);
        m_all_ok &= (status == PSA_SUCCESS);

        if (m_verbose) printf("Destroying encryption key: %d", m_encryption_key_id);
        status = m_crypto_client->destroy_key(m_encryption_key_id);
        print_status(status);
        m_all_ok &= (status == PSA_SUCCESS);
    }

private:

    static const psa_key_id_t SIGNING_KEY_ID = 0x100;
    static const psa_key_id_t ENCRYPTION_KEY_ID = 0x101;

    crypto_client *m_crypto_client;
    psa_key_id_t m_signing_key_id;
    psa_key_id_t m_encryption_key_id;

    bool m_verbose;
    bool m_all_ok;
};


int run_ts_demo(crypto_client *crypto_client, bool is_verbose) {

    ts_demo demo(crypto_client, is_verbose);

    demo.print_intro();
    demo.wait(1);
    demo.generate_random_number(1);
    demo.wait(1);
    demo.generate_random_number(7);
    demo.wait(1);
    demo.generate_random_number(128);
    demo.wait(1);
    demo.generate_signing_key();
    demo.wait(2);
    demo.sign_and_verify_message("The quick brown fox");
    demo.wait(3);
    demo.sign_and_verify_message("jumps over the lazy dog");
    demo.wait(3);
    demo.generate_asymmetric_encryption_key();
    demo.wait(2);
    demo.encrypt_add_decrypt_message("Top secret");
    demo.wait(3);
    demo.export_public_key();
    demo.wait(2);
    demo.destroy_keys();
    demo.wait(2);

    return demo.is_all_ok() ? 0 : -1;
}
