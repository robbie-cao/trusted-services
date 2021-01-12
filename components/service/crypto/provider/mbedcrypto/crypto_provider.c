/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include <stdlib.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <service/crypto/provider/mbedcrypto/crypto_provider.h>
#include <service/crypto/provider/serializer/crypto_provider_serializer.h>
#include <service/crypto/provider/serializer/protobuf/pb_crypto_provider_serializer.h>
#include <service/secure_storage/client/psa/its/its_client.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <psa/crypto.h>

/* Service request handlers */
static rpc_status_t nop_handler(void *context, struct call_req* req);
static rpc_status_t generate_key_handler(void *context, struct call_req* req);
static rpc_status_t destroy_key_handler(void *context, struct call_req* req);
static rpc_status_t open_key_handler(void *context, struct call_req* req);
static rpc_status_t close_key_handler(void *context, struct call_req* req);
static rpc_status_t export_key_handler(void *context, struct call_req* req);
static rpc_status_t export_public_key_handler(void *context, struct call_req* req);
static rpc_status_t import_key_handler(void *context, struct call_req* req);
static rpc_status_t sign_hash_handler(void *context, struct call_req* req);
static rpc_status_t verify_hash_handler(void *context, struct call_req* req);
static rpc_status_t asymmetric_decrypt_handler(void *context, struct call_req* req);
static rpc_status_t asymmetric_encrypt_handler(void *context, struct call_req* req);
static rpc_status_t generate_random_handler(void *context, struct call_req* req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
    {TS_CRYPTO_OPCODE_NOP,                  nop_handler},
    {TS_CRYPTO_OPCODE_GENERATE_KEY,         generate_key_handler},
    {TS_CRYPTO_OPCODE_DESTROY_KEY,          destroy_key_handler},
    {TS_CRYPTO_OPCODE_OPEN_KEY,             open_key_handler},
    {TS_CRYPTO_OPCODE_CLOSE_KEY,            close_key_handler},
    {TS_CRYPTO_OPCODE_EXPORT_KEY,           export_key_handler},
    {TS_CRYPTO_OPCODE_EXPORT_PUBLIC_KEY,    export_public_key_handler},
    {TS_CRYPTO_OPCODE_IMPORT_KEY,           import_key_handler},
    {TS_CRYPTO_OPCODE_SIGN_HASH,            sign_hash_handler},
    {TS_CRYPTO_OPCODE_VERIFY_HASH,          verify_hash_handler},
    {TS_CRYPTO_OPCODE_ASYMMETRIC_DECRYPT,   asymmetric_decrypt_handler},
    {TS_CRYPTO_OPCODE_ASYMMETRIC_ENCRYPT,   asymmetric_encrypt_handler},
    {TS_CRYPTO_OPCODE_GENERATE_RANDOM,      generate_random_handler}
};

struct call_ep *mbed_crypto_provider_init(struct mbed_crypto_provider *context,
                                        struct rpc_caller *storage_provider)
{
    struct call_ep *call_ep = NULL;

    /*
     * A storage provider is required for persistent key storage.  As this
     * is a mandatory feature of the crypto service, insist on a storage
     * provider being available.
     */
    if (context && storage_provider) {

        service_provider_init(&context->base_provider, context,
                    handler_table, sizeof(handler_table)/sizeof(struct service_handler));

        service_set_default_serializer(&context->base_provider,
                    pb_crypto_provider_serializer_instance());

        if ((psa_its_client_init(storage_provider) == PSA_SUCCESS) &&
            (psa_crypto_init() == PSA_SUCCESS))
            call_ep = service_provider_get_call_ep(&context->base_provider);
    }

    return call_ep;
}

void mbed_crypto_provider_deinit(struct mbed_crypto_provider *context)
{
    (void)context;
}

static inline const struct crypto_provider_serializer* get_crypto_serializer(const struct call_req *req)
{
    return (const struct crypto_provider_serializer*)call_req_get_serializer(req);
}

static rpc_status_t nop_handler(void *context, struct call_req* req)
{
    /* Responds to a request by returning success */
    rpc_status_t rpc_status = TS_RPC_CALL_ACCEPTED;
    psa_status_t psa_status = PSA_SUCCESS;

    (void)context;
    call_req_set_opstatus(req, psa_status);

    return rpc_status;
}

static rpc_status_t generate_key_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
    (void)context;

    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    rpc_status = serializer->deserialize_generate_key_req(req_buf, &attributes);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        psa_status_t psa_status;
        psa_key_handle_t handle;

        psa_status = psa_generate_key(&attributes, &handle);

        if (psa_status == PSA_SUCCESS) {

            struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
            rpc_status = serializer->serialize_generate_key_resp(resp_buf, handle);
        }

        call_req_set_opstatus(req, psa_status);
    }

    psa_reset_key_attributes(&attributes);

    return rpc_status;
}

static rpc_status_t destroy_key_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
	(void)context;

    psa_key_handle_t handle;

    rpc_status = serializer->deserialize_destroy_key_req(req_buf, &handle);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        psa_status_t psa_status;

        psa_status = psa_destroy_key(handle);
        call_req_set_opstatus(req, psa_status);
    }

    return rpc_status;
}

static rpc_status_t open_key_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
    (void)context;

    psa_key_id_t id;

    rpc_status = serializer->deserialize_open_key_req(req_buf, &id);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        psa_status_t psa_status;
        psa_key_handle_t handle;

        psa_status = psa_open_key(id, &handle);

        if (psa_status == PSA_SUCCESS) {

            struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
            rpc_status = serializer->serialize_open_key_resp(resp_buf, handle);
        }

        call_req_set_opstatus(req, psa_status);
    }

    return rpc_status;
}

static rpc_status_t close_key_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
	(void)context;

    psa_key_handle_t handle;

    rpc_status = serializer->deserialize_close_key_req(req_buf, &handle);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        psa_status_t psa_status;

        psa_status = psa_close_key(handle);
        call_req_set_opstatus(req, psa_status);
    }

    return rpc_status;
}

static rpc_status_t export_key_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
    (void)context;

    psa_key_handle_t handle;

    rpc_status = serializer->deserialize_export_key_req(req_buf, &handle);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        psa_status_t psa_status;
        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

        psa_status = psa_get_key_attributes(handle, &attributes);

        if (psa_status == PSA_SUCCESS) {

            size_t max_export_size = PSA_KEY_EXPORT_MAX_SIZE(
                psa_get_key_type(&attributes),
                psa_get_key_bits(&attributes));

            uint8_t *key_buffer = malloc(max_export_size);

            if (key_buffer) {

                size_t export_size;
                psa_status = psa_export_key(handle, key_buffer, max_export_size, &export_size);

                if (psa_status == PSA_SUCCESS) {

                    struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
                    rpc_status = serializer->serialize_export_key_resp(resp_buf, key_buffer, export_size);
                }

                free(key_buffer);
            }
            else {
                /* Failed to allocate key buffer */
                rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
            }
        }

        call_req_set_opstatus(req, psa_status);
        psa_reset_key_attributes(&attributes);
    }

    return rpc_status;
}

static rpc_status_t export_public_key_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
    (void)context;

    psa_key_handle_t handle;

    rpc_status = serializer->deserialize_export_public_key_req(req_buf, &handle);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        psa_status_t psa_status;
        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

        psa_status = psa_get_key_attributes(handle, &attributes);

        if (psa_status == PSA_SUCCESS) {

            size_t max_export_size = PSA_KEY_EXPORT_MAX_SIZE(
                PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR(psa_get_key_type(&attributes)),
                psa_get_key_bits(&attributes));

            uint8_t *key_buffer = malloc(max_export_size);

            if (key_buffer) {

                size_t export_size;
                psa_status = psa_export_public_key(handle, key_buffer, max_export_size, &export_size);

                if (psa_status == PSA_SUCCESS) {

                    struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
                    rpc_status = serializer->serialize_export_public_key_resp(resp_buf, key_buffer, export_size);
                }

                free(key_buffer);
            }
            else {
                /* Failed to allocate key buffer */
                rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
            }
        }

        call_req_set_opstatus(req, psa_status);
        psa_reset_key_attributes(&attributes);
    }

    return rpc_status;
}

static rpc_status_t import_key_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
    (void)context;

    size_t key_data_len = serializer->max_deserialised_parameter_size(req_buf);
    uint8_t *key_buffer = malloc(key_data_len);

    if (key_buffer) {

        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
        rpc_status = serializer->deserialize_import_key_req(req_buf, &attributes, key_buffer, &key_data_len);

        if (rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status_t psa_status;
            psa_key_handle_t handle;

            psa_status = psa_import_key(&attributes, key_buffer, key_data_len, &handle);

            if (psa_status == PSA_SUCCESS) {

                struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
                rpc_status = serializer->serialize_import_key_resp(resp_buf, handle);
            }

            call_req_set_opstatus(req, psa_status);
        }

        psa_reset_key_attributes(&attributes);
        free(key_buffer);
    }
    else {

        rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
    }

    return rpc_status;
}

static rpc_status_t sign_hash_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
    (void)context;

    psa_key_handle_t handle;
    psa_algorithm_t alg;
    size_t hash_len = PSA_HASH_MAX_SIZE;
    uint8_t hash_buffer[PSA_HASH_MAX_SIZE];

    rpc_status = serializer->deserialize_sign_hash_req(req_buf, &handle, &alg, hash_buffer, &hash_len);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        psa_status_t psa_status;
        size_t sig_len;
        uint8_t sig_buffer[PSA_SIGNATURE_MAX_SIZE];

        psa_status = psa_sign_hash(handle, alg,
                    hash_buffer, hash_len,
                    sig_buffer, sizeof(sig_buffer), &sig_len);

        if (psa_status == PSA_SUCCESS) {

            struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
            rpc_status = serializer->serialize_sign_hash_resp(resp_buf, sig_buffer, sig_len);
        }

        call_req_set_opstatus(req, psa_status);
    }

    return rpc_status;
}

static rpc_status_t verify_hash_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
    (void)context;

    psa_key_handle_t handle;
    psa_algorithm_t alg;
    size_t hash_len = PSA_HASH_MAX_SIZE;
    uint8_t hash_buffer[PSA_HASH_MAX_SIZE];
    size_t sig_len = PSA_SIGNATURE_MAX_SIZE;
    uint8_t sig_buffer[PSA_SIGNATURE_MAX_SIZE];

    rpc_status = serializer->deserialize_verify_hash_req(req_buf, &handle, &alg,
                                            hash_buffer, &hash_len,
                                            sig_buffer, &sig_len);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        psa_status_t psa_status;

        psa_status = psa_verify_hash(handle, alg,
                    hash_buffer, hash_len,
                    sig_buffer, sig_len);

        call_req_set_opstatus(req, psa_status);
    }

    return rpc_status;
}

static rpc_status_t asymmetric_decrypt_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
    size_t max_param_size = serializer->max_deserialised_parameter_size(req_buf);
    (void)context;

    psa_key_handle_t handle;
    psa_algorithm_t alg;
    size_t ciphertext_len = max_param_size;
    uint8_t *ciphertext_buffer = malloc(ciphertext_len);
    size_t salt_len = max_param_size;
    uint8_t *salt_buffer = malloc(salt_len);

    if (ciphertext_buffer && salt_buffer) {

        rpc_status = serializer->deserialize_asymmetric_decrypt_req(req_buf,
                                                &handle, &alg,
                                                ciphertext_buffer, &ciphertext_len,
                                                salt_buffer, &salt_len);

        if (rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status_t psa_status;
            psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

            psa_status = psa_get_key_attributes(handle, &attributes);

            if (psa_status == PSA_SUCCESS) {

                size_t max_decrypt_size = PSA_ASYMMETRIC_DECRYPT_OUTPUT_SIZE(
                    psa_get_key_type(&attributes),
                    psa_get_key_bits(&attributes),
                    alg);

                size_t plaintext_len;
                uint8_t *plaintext_buffer = malloc(max_decrypt_size);

                if (plaintext_buffer) {

                    psa_status = psa_asymmetric_decrypt(handle, alg,
                                ciphertext_buffer, ciphertext_len,
                                salt_buffer, salt_len,
                                plaintext_buffer, max_decrypt_size, &plaintext_len);

                    if (psa_status == PSA_SUCCESS) {

                        struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
                        rpc_status = serializer->serialize_asymmetric_decrypt_resp(resp_buf,
                                                            plaintext_buffer, plaintext_len);
                    }

                    free(plaintext_buffer);
                }
                else {
                    /* Failed to allocate ouptput buffer */
                    rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
                }
            }

            call_req_set_opstatus(req, psa_status);
            psa_reset_key_attributes(&attributes);
        }
    }
    else {
        /* Failed to allocate buffers */
        rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
    }

    free(ciphertext_buffer);
    free(salt_buffer);

    return rpc_status;
}

static rpc_status_t asymmetric_encrypt_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
    size_t max_param_size = serializer->max_deserialised_parameter_size(req_buf);
    (void)context;

    psa_key_handle_t handle;
    psa_algorithm_t alg;
    size_t plaintext_len = max_param_size;
    uint8_t *plaintext_buffer = malloc(plaintext_len);
    size_t salt_len = max_param_size;
    uint8_t *salt_buffer = malloc(salt_len);

    if (plaintext_buffer && salt_buffer) {

        rpc_status = serializer->deserialize_asymmetric_encrypt_req(req_buf,
                                                &handle, &alg,
                                                plaintext_buffer, &plaintext_len,
                                                salt_buffer, &salt_len);

        if (rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status_t psa_status;
            psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

            psa_status = psa_get_key_attributes(handle, &attributes);

            if (psa_status == PSA_SUCCESS) {

                size_t max_encrypt_size = PSA_ASYMMETRIC_ENCRYPT_OUTPUT_SIZE(
                    psa_get_key_type(&attributes),
                    psa_get_key_bits(&attributes),
                    alg);

                size_t ciphertext_len;
                uint8_t *ciphertext_buffer = malloc(max_encrypt_size);

                if (ciphertext_buffer) {

                    psa_status = psa_asymmetric_encrypt(handle, alg,
                                plaintext_buffer, plaintext_len,
                                salt_buffer, salt_len,
                                ciphertext_buffer, max_encrypt_size, &ciphertext_len);

                    if (psa_status == PSA_SUCCESS) {

                        struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
                        rpc_status = serializer->serialize_asymmetric_encrypt_resp(resp_buf,
                                                            ciphertext_buffer, ciphertext_len);
                    }

                    free(ciphertext_buffer);
                }
                else {
                    /* Failed to allocate ouptput buffer */
                    rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
                }
            }

            call_req_set_opstatus(req, psa_status);
            psa_reset_key_attributes(&attributes);
        }
    }
    else {
        /* Failed to allocate buffers */
        rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
    }

    free(plaintext_buffer);
    free(salt_buffer);

    return rpc_status;
}

static rpc_status_t generate_random_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct crypto_provider_serializer *serializer = get_crypto_serializer(req);
    (void)context;

    size_t output_size;

    rpc_status = serializer->deserialize_generate_random_req(req_buf, &output_size);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        psa_status_t psa_status;
        uint8_t *output_buffer = malloc(output_size);

        if (output_buffer) {

            psa_status = psa_generate_random(output_buffer, output_size);

            if (psa_status == PSA_SUCCESS) {

                struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
                rpc_status = serializer->serialize_generate_random_resp(resp_buf,
                                                    output_buffer, output_size);
            }

            call_req_set_opstatus(req, psa_status);
            free(output_buffer);
        }
        else {
            /* Failed to allocate output buffer */
            rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
        }
    }

    return rpc_status;
}