/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CRYPTO_CLIENT_H
#define CRYPTO_CLIENT_H

#include <cstdint>
#include <psa/crypto.h>
#include <service/crypto/protobuf/key_attributes.pb.h>


struct rpc_caller;

/** Provides a client interface for accessing an instance of the PSA Crypto service.
 **/
class crypto_client
{
public:
    crypto_client(struct rpc_caller *caller);
    virtual ~crypto_client();

    int err_rpc_status() const;

    /* Key lifecycle methods */
    psa_status_t generate_key(const psa_key_attributes_t *attributes, psa_key_handle_t *handle);
    psa_status_t destroy_key(psa_key_handle_t handle);
    psa_status_t open_key(psa_key_id_t id, psa_key_handle_t *handle);
    psa_status_t close_key(psa_key_handle_t handle);
    psa_status_t import_key(const psa_key_attributes_t *attributes,
                            const uint8_t *data, size_t data_length, psa_key_handle_t *handle);

    /* Key export methods */
    psa_status_t export_key(psa_key_handle_t handle,
                            uint8_t *data, size_t data_size,
                            size_t *data_length);
    psa_status_t export_public_key(psa_key_handle_t handle,
                            uint8_t *data, size_t data_size, size_t *data_length);

    /* Sign/verify methods */
    psa_status_t sign_hash(psa_key_handle_t handle, psa_algorithm_t alg,
                            const uint8_t *hash, size_t hash_length,
                            uint8_t *signature, size_t signature_size, size_t *signature_length);
    psa_status_t verify_hash(psa_key_handle_t handle, psa_algorithm_t alg,
                            const uint8_t *hash, size_t hash_length,
                            const uint8_t *signature, size_t signature_length);

    /* Asymmetric encrypt/decrypt */
    psa_status_t asymmetric_encrypt(psa_key_handle_t handle, psa_algorithm_t alg,
                            const uint8_t *input, size_t input_length,
                            const uint8_t *salt, size_t salt_length,
                            uint8_t *output, size_t output_size, size_t *output_length);
    psa_status_t asymmetric_decrypt(psa_key_handle_t handle, psa_algorithm_t alg,
                            const uint8_t *input, size_t input_length,
                            const uint8_t *salt, size_t salt_length,
                            uint8_t *output, size_t output_size, size_t *output_length);

    /* Random number generation */
    psa_status_t generate_random(uint8_t *output, size_t output_size);

protected:
    crypto_client();
    void set_caller(struct rpc_caller *caller);

private:

    void translate_key_attributes(ts_crypto_KeyAttributes &proto_attributes,
                            const psa_key_attributes_t &psa_attributes);

    struct rpc_caller *m_caller;
    int m_err_rpc_status;
};

#endif /* CRYPTO_CLIENT_H */
