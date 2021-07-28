/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CRYPTO_CLIENT_H
#define CRYPTO_CLIENT_H

#include <cstdint>
#include <psa/crypto.h>
#include <service/common/client/service_client.h>

/*
 * Provides a client interface for accessing an instance of the Crypto service
 * using a C++ version of the PSA Crypto API.
 */
class crypto_client
{
public:
    virtual ~crypto_client();

    int err_rpc_status() const;

    /* Key lifecycle methods */
    virtual psa_status_t generate_key(const psa_key_attributes_t *attributes,
                            psa_key_id_t *id) = 0;
    virtual psa_status_t destroy_key(psa_key_id_t id) = 0;
    virtual psa_status_t import_key(const psa_key_attributes_t *attributes,
                            const uint8_t *data, size_t data_length, psa_key_id_t *id) = 0;

    /* Key export methods */
    virtual psa_status_t export_key(psa_key_id_t id,
                            uint8_t *data, size_t data_size,
                            size_t *data_length) = 0;
    virtual psa_status_t export_public_key(psa_key_id_t id,
                            uint8_t *data, size_t data_size, size_t *data_length) = 0;

    /* Sign/verify methods */
    virtual psa_status_t sign_hash(psa_key_id_t id, psa_algorithm_t alg,
                            const uint8_t *hash, size_t hash_length,
                            uint8_t *signature, size_t signature_size,
                            size_t *signature_length) = 0;
    virtual psa_status_t verify_hash(psa_key_id_t id, psa_algorithm_t alg,
                            const uint8_t *hash, size_t hash_length,
                            const uint8_t *signature, size_t signature_length) = 0;

    /* Asymmetric encrypt/decrypt */
    virtual psa_status_t asymmetric_encrypt(psa_key_id_t id, psa_algorithm_t alg,
                            const uint8_t *input, size_t input_length,
                            const uint8_t *salt, size_t salt_length,
                            uint8_t *output, size_t output_size, size_t *output_length) = 0;
    virtual psa_status_t asymmetric_decrypt(psa_key_id_t id, psa_algorithm_t alg,
                            const uint8_t *input, size_t input_length,
                            const uint8_t *salt, size_t salt_length,
                            uint8_t *output, size_t output_size, size_t *output_length) = 0;

    /* Random number generation */
    virtual psa_status_t generate_random(uint8_t *output, size_t output_size) = 0;

    /* Hash methods */
    virtual psa_status_t hash_setup(uint32_t *op_handle,
                            psa_algorithm_t alg) = 0;
    virtual psa_status_t hash_update(uint32_t op_handle,
                            const uint8_t *input, size_t input_length) = 0;
    virtual psa_status_t hash_finish(uint32_t op_handle,
                            uint8_t *hash, size_t hash_size, size_t *hash_length) = 0;

protected:
    crypto_client();
    crypto_client(struct rpc_caller *caller);
    void set_caller(struct rpc_caller *caller);

    struct service_client m_client;
};

#endif /* CRYPTO_CLIENT_H */
