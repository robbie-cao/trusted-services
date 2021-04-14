/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "psa_crypto_api_client.h"
#include <service/crypto/client/psa/psa_crypto_client.h>


psa_crypto_api_client::psa_crypto_api_client() :
    crypto_client()
{

}

psa_crypto_api_client::~psa_crypto_api_client()
{

}

psa_status_t psa_crypto_api_client::generate_key(const psa_key_attributes_t *attributes, psa_key_id_t *id)
{
    psa_status_t psa_status = psa_generate_key(attributes, id);
    m_err_rpc_status = psa_crypto_client_rpc_status();

    return psa_status;
}

psa_status_t psa_crypto_api_client::destroy_key(psa_key_id_t id)
{
    psa_status_t psa_status = psa_destroy_key(id);
    m_err_rpc_status = psa_crypto_client_rpc_status();

    return psa_status;
}

psa_status_t psa_crypto_api_client::import_key(const psa_key_attributes_t *attributes,
                        const uint8_t *data, size_t data_length, psa_key_id_t *id)
{
    psa_status_t psa_status = psa_import_key(attributes, data, data_length, id);
    m_err_rpc_status = psa_crypto_client_rpc_status();

    return psa_status;
}

psa_status_t psa_crypto_api_client::export_key(psa_key_id_t id,
                        uint8_t *data, size_t data_size,
                        size_t *data_length)
{
    psa_status_t psa_status = psa_export_key(id, data, data_size, data_length);
    m_err_rpc_status = psa_crypto_client_rpc_status();

    return psa_status;
}

psa_status_t psa_crypto_api_client::export_public_key(psa_key_id_t id,
                                uint8_t *data, size_t data_size, size_t *data_length)
{
    psa_status_t psa_status = psa_export_public_key(id, data, data_size, data_length);
    m_err_rpc_status = psa_crypto_client_rpc_status();

    return psa_status;
}

psa_status_t psa_crypto_api_client::sign_hash(psa_key_id_t id, psa_algorithm_t alg,
                            const uint8_t *hash, size_t hash_length,
                            uint8_t *signature, size_t signature_size, size_t *signature_length)
{
    psa_status_t psa_status = psa_sign_hash(id, alg, hash, hash_length,
                                    signature, signature_size, signature_length);
    m_err_rpc_status = psa_crypto_client_rpc_status();

    return psa_status;
}


psa_status_t psa_crypto_api_client::verify_hash(psa_key_id_t id, psa_algorithm_t alg,
                        const uint8_t *hash, size_t hash_length,
                        const uint8_t *signature, size_t signature_length)
{
    psa_status_t psa_status = psa_verify_hash(id, alg, hash, hash_length,
                                    signature, signature_length);
    m_err_rpc_status = psa_crypto_client_rpc_status();

    return psa_status;
}

psa_status_t psa_crypto_api_client::asymmetric_encrypt(psa_key_id_t id, psa_algorithm_t alg,
                        const uint8_t *input, size_t input_length,
                        const uint8_t *salt, size_t salt_length,
                        uint8_t *output, size_t output_size, size_t *output_length)
{
    psa_status_t psa_status = psa_asymmetric_encrypt(id, alg, input, input_length,
                                                salt, salt_length,
                                                output, output_size, output_length);
    m_err_rpc_status = psa_crypto_client_rpc_status();

    return psa_status;
}

psa_status_t psa_crypto_api_client::asymmetric_decrypt(psa_key_id_t id, psa_algorithm_t alg,
                        const uint8_t *input, size_t input_length,
                        const uint8_t *salt, size_t salt_length,
                        uint8_t *output, size_t output_size, size_t *output_length)
{
    psa_status_t psa_status = psa_asymmetric_decrypt(id, alg, input, input_length,
                                                salt, salt_length,
                                                output, output_size, output_length);
    m_err_rpc_status = psa_crypto_client_rpc_status();

    return psa_status;
}

psa_status_t psa_crypto_api_client::generate_random(uint8_t *output, size_t output_size)
{
    psa_status_t psa_status = psa_generate_random(output, output_size);
    m_err_rpc_status = psa_crypto_client_rpc_status();

    return psa_status;
}
