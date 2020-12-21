/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstring>
#include <cstdlib>
#include "packedc_crypto_client.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <protocols/service/crypto/packed-c/key_attributes.h>
#include <protocols/service/crypto/packed-c/asymmetric_decrypt.h>
#include <protocols/service/crypto/packed-c/asymmetric_encrypt.h>
#include <protocols/service/crypto/packed-c/close_key.h>
#include <protocols/service/crypto/packed-c/destroy_key.h>
#include <protocols/service/crypto/packed-c/export_key.h>
#include <protocols/service/crypto/packed-c/export_public_key.h>
#include <protocols/service/crypto/packed-c/generate_key.h>
#include <protocols/service/crypto/packed-c/generate_random.h>
#include <protocols/service/crypto/packed-c/import_key.h>
#include <protocols/service/crypto/packed-c/open_key.h>
#include <protocols/service/crypto/packed-c/sign_hash.h>
#include <protocols/service/crypto/packed-c/verify_hash.h>
#include <common/tlv/tlv.h>
#include <rpc_caller.h>


packedc_crypto_client::packedc_crypto_client() :
    crypto_client()
{

}

packedc_crypto_client::packedc_crypto_client(struct rpc_caller *caller) :
    crypto_client(caller)
{

}

packedc_crypto_client::~packedc_crypto_client()
{

}

psa_status_t packedc_crypto_client::generate_key(const psa_key_attributes_t *attributes, psa_key_handle_t *handle)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_generate_key_in req_msg;
    size_t req_len = sizeof(ts_crypto_generate_key_in);

    translate_key_attributes(req_msg.attributes, *attributes);

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;

        memcpy(req_buf, &req_msg, req_len);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
            TS_CRYPTO_OPCODE_GENERATE_KEY, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                if (resp_len >= sizeof(ts_crypto_generate_key_out)) {

                    struct ts_crypto_generate_key_out resp_msg;
                    memcpy(&resp_msg, resp_buf, sizeof(ts_crypto_generate_key_out));
                    *handle = resp_msg.handle;
                }
                else {
                    /* Failed to decode response message */
                    psa_status = PSA_ERROR_GENERIC_ERROR;
                }
            }
        }

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}

psa_status_t packedc_crypto_client::destroy_key(psa_key_handle_t handle)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_destroy_key_in req_msg;
    size_t req_len = sizeof(ts_crypto_destroy_key_in);

    req_msg.handle = handle;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;

        memcpy(req_buf, &req_msg, req_len);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
            TS_CRYPTO_OPCODE_DESTROY_KEY, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}

psa_status_t packedc_crypto_client::open_key(psa_key_id_t id, psa_key_handle_t *handle)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_open_key_in req_msg;
    size_t req_len = sizeof(ts_crypto_open_key_in);

    req_msg.id = id;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;

        memcpy(req_buf, &req_msg, req_len);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
                    TS_CRYPTO_OPCODE_OPEN_KEY, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                if (resp_len >= sizeof(ts_crypto_open_key_out)) {

                    struct ts_crypto_open_key_out resp_msg;
                    memcpy(&resp_msg, resp_buf, sizeof(ts_crypto_open_key_out));
                    *handle = resp_msg.handle;
                }
                else {
                    /* Failed to decode response message */
                    psa_status = PSA_ERROR_GENERIC_ERROR;
                }
            }
        }

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}

psa_status_t packedc_crypto_client::close_key(psa_key_handle_t handle)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_close_key_in req_msg;
    size_t req_len = sizeof(ts_crypto_close_key_in);

    req_msg.handle = handle;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;

        memcpy(req_buf, &req_msg, req_len);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
            TS_CRYPTO_OPCODE_CLOSE_KEY, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}

psa_status_t packedc_crypto_client::import_key(const psa_key_attributes_t *attributes,
                        const uint8_t *data, size_t data_length, psa_key_handle_t *handle)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_import_key_in req_msg;
    size_t req_fixed_len = sizeof(ts_crypto_import_key_in);
    size_t req_len = req_fixed_len + tlv_required_space(data_length);

    translate_key_attributes(req_msg.attributes, *attributes);

    struct tlv_record key_record;
    key_record.tag = TS_CRYPTO_IMPORT_KEY_IN_TAG_DATA;
    key_record.length = data_length;
    key_record.value = data;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;
        struct tlv_iterator req_iter;

        memcpy(req_buf, &req_msg, req_fixed_len);

        tlv_iterator_begin(&req_iter, &req_buf[req_fixed_len], req_len - req_fixed_len);
        tlv_encode(&req_iter, &key_record);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
            TS_CRYPTO_OPCODE_IMPORT_KEY, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                if (resp_len >= sizeof(ts_crypto_open_key_out)) {

                    struct ts_crypto_import_key_out resp_msg;
                    memcpy(&resp_msg, resp_buf, sizeof(ts_crypto_import_key_out));
                    *handle = resp_msg.handle;
                }
                else {
                    /* Failed to decode response message */
                    psa_status = PSA_ERROR_GENERIC_ERROR;
                }
            }
        }

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}

psa_status_t packedc_crypto_client::export_key(psa_key_handle_t handle,
                        uint8_t *data, size_t data_size,
                        size_t *data_length)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_export_key_in req_msg;
    size_t req_len = sizeof(ts_crypto_export_key_in);

    req_msg.handle = handle;

    *data_length = 0; /* For failure case */

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;

        memcpy(req_buf, &req_msg, req_len);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
            TS_CRYPTO_OPCODE_EXPORT_KEY, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                struct tlv_const_iterator resp_iter;
                struct tlv_record decoded_record;
                tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

                if (tlv_find_decode(&resp_iter, TS_CRYPTO_EXPORT_KEY_OUT_TAG_DATA, &decoded_record)) {

                    if (decoded_record.length <= data_size) {

                        memcpy(data, decoded_record.value, decoded_record.length);
                        *data_length = decoded_record.length;
                    }
                    else {
                        /* Provided buffer is too small */
                        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
                    }
                }
                else {
                    /* Mandatory response parameter missing */
                    psa_status = PSA_ERROR_GENERIC_ERROR;
                }
            }
        }

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}

psa_status_t packedc_crypto_client::export_public_key(psa_key_handle_t handle,
                                uint8_t *data, size_t data_size, size_t *data_length)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_export_public_key_in req_msg;
    size_t req_len = sizeof(ts_crypto_export_public_key_in);

    req_msg.handle = handle;

    *data_length = 0; /* For failure case */

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;

        memcpy(req_buf, &req_msg, req_len);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
            TS_CRYPTO_OPCODE_EXPORT_PUBLIC_KEY, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                struct tlv_const_iterator resp_iter;
                struct tlv_record decoded_record;
                tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

                if (tlv_find_decode(&resp_iter, TS_CRYPTO_EXPORT_PUBLIC_KEY_OUT_TAG_DATA, &decoded_record)) {

                    if (decoded_record.length <= data_size) {

                        memcpy(data, decoded_record.value, decoded_record.length);
                        *data_length = decoded_record.length;
                    }
                    else {
                        /* Provided buffer is too small */
                        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
                    }
                }
                else {
                    /* Mandatory response parameter missing */
                    psa_status = PSA_ERROR_GENERIC_ERROR;
                }
            }
        }

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}

psa_status_t packedc_crypto_client::sign_hash(psa_key_handle_t handle, psa_algorithm_t alg,
                            const uint8_t *hash, size_t hash_length,
                            uint8_t *signature, size_t signature_size, size_t *signature_length)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_sign_hash_in req_msg;
    size_t req_fixed_len = sizeof(ts_crypto_sign_hash_in);
    size_t req_len = req_fixed_len + tlv_required_space(hash_length);

    *signature_length = 0;  /* For failure case */

    req_msg.handle = handle;
    req_msg.alg = alg;

    struct tlv_record hash_record;
    hash_record.tag = TS_CRYPTO_SIGN_HASH_IN_TAG_HASH;
    hash_record.length = hash_length;
    hash_record.value = hash;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;
        struct tlv_iterator req_iter;

        memcpy(req_buf, &req_msg, req_fixed_len);

        tlv_iterator_begin(&req_iter, &req_buf[req_fixed_len], req_len - req_fixed_len);
        tlv_encode(&req_iter, &hash_record);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
                    TS_CRYPTO_OPCODE_SIGN_HASH, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                struct tlv_const_iterator resp_iter;
                struct tlv_record decoded_record;
                tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

                if (tlv_find_decode(&resp_iter, TS_CRYPTO_SIGN_HASH_OUT_TAG_SIGNATURE, &decoded_record)) {

                    if (decoded_record.length <= signature_size) {

                        memcpy(signature, decoded_record.value, decoded_record.length);
                        *signature_length = decoded_record.length;
                    }
                    else {
                        /* Provided buffer is too small */
                        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
                    }
                }
                else {
                    /* Mandatory response parameter missing */
                    psa_status = PSA_ERROR_GENERIC_ERROR;
                }
            }
        }

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}


psa_status_t packedc_crypto_client::verify_hash(psa_key_handle_t handle, psa_algorithm_t alg,
                        const uint8_t *hash, size_t hash_length,
                        const uint8_t *signature, size_t signature_length)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_verify_hash_in req_msg;
    size_t req_fixed_len = sizeof(ts_crypto_verify_hash_in);
    size_t req_len = req_fixed_len + tlv_required_space(hash_length) + tlv_required_space(signature_length);

    req_msg.handle = handle;
    req_msg.alg = alg;

    struct tlv_record hash_record;
    hash_record.tag = TS_CRYPTO_VERIFY_HASH_IN_TAG_HASH;
    hash_record.length = hash_length;
    hash_record.value = hash;

    struct tlv_record sig_record;
    sig_record.tag = TS_CRYPTO_VERIFY_HASH_IN_TAG_SIGNATURE;
    sig_record.length = signature_length;
    sig_record.value = signature;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;
        struct tlv_iterator req_iter;

        memcpy(req_buf, &req_msg, req_fixed_len);

        tlv_iterator_begin(&req_iter, &req_buf[req_fixed_len], req_len - req_fixed_len);
        tlv_encode(&req_iter, &hash_record);
        tlv_encode(&req_iter, &sig_record);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
                    TS_CRYPTO_OPCODE_VERIFY_HASH, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) psa_status = opstatus;

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}

psa_status_t packedc_crypto_client::asymmetric_encrypt(psa_key_handle_t handle, psa_algorithm_t alg,
                        const uint8_t *input, size_t input_length,
                        const uint8_t *salt, size_t salt_length,
                        uint8_t *output, size_t output_size, size_t *output_length)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_asymmetric_encrypt_in req_msg;
    size_t req_fixed_len = sizeof(ts_crypto_asymmetric_encrypt_in);
    size_t req_len = req_fixed_len + tlv_required_space(input_length) + tlv_required_space(salt_length);

    *output_length = 0;  /* For failure case */

    req_msg.handle = handle;
    req_msg.alg = alg;

    struct tlv_record plaintext_record;
    plaintext_record.tag = TS_CRYPTO_ASYMMETRIC_ENCRYPT_IN_TAG_PLAINTEXT;
    plaintext_record.length = input_length;
    plaintext_record.value = input;

    struct tlv_record salt_record;
    salt_record.tag = TS_CRYPTO_ASYMMETRIC_ENCRYPT_IN_TAG_SALT;
    salt_record.length = salt_length;
    salt_record.value = salt;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus = PSA_ERROR_GENERIC_ERROR;
        struct tlv_iterator req_iter;

        memcpy(req_buf, &req_msg, req_fixed_len);

        tlv_iterator_begin(&req_iter, &req_buf[req_fixed_len], req_len - req_fixed_len);
        tlv_encode(&req_iter, &plaintext_record);
        tlv_encode(&req_iter, &salt_record);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
                    TS_CRYPTO_OPCODE_ASYMMETRIC_ENCRYPT, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                struct tlv_const_iterator resp_iter;
                struct tlv_record decoded_record;
                tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

                if (tlv_find_decode(&resp_iter, TS_CRYPTO_ASYMMETRIC_ENCRYPT_OUT_TAG_CIPHERTEXT, &decoded_record)) {

                    if (decoded_record.length <= output_size) {

                        memcpy(output, decoded_record.value, decoded_record.length);
                        *output_length = decoded_record.length;
                    }
                    else {
                        /* Provided buffer is too small */
                        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
                    }
                }
                else {
                    /* Mandatory response parameter missing */
                    psa_status = PSA_ERROR_GENERIC_ERROR;
                }
            }
        }

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}

psa_status_t packedc_crypto_client::asymmetric_decrypt(psa_key_handle_t handle, psa_algorithm_t alg,
                        const uint8_t *input, size_t input_length,
                        const uint8_t *salt, size_t salt_length,
                        uint8_t *output, size_t output_size, size_t *output_length)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_asymmetric_decrypt_in req_msg;
    size_t req_fixed_len = sizeof(ts_crypto_asymmetric_decrypt_in);
    size_t req_len = req_fixed_len + tlv_required_space(input_length) + tlv_required_space(salt_length);

    *output_length = 0;  /* For failure case */

    req_msg.handle = handle;
    req_msg.alg = alg;

    struct tlv_record ciphertext_record;
    ciphertext_record.tag = TS_CRYPTO_ASYMMETRIC_DECRYPT_IN_TAG_CIPHERTEXT;
    ciphertext_record.length = input_length;
    ciphertext_record.value = input;

    struct tlv_record salt_record;
    salt_record.tag = TS_CRYPTO_ASYMMETRIC_DECRYPT_IN_TAG_SALT;
    salt_record.length = salt_length;
    salt_record.value = salt;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;
        struct tlv_iterator req_iter;

        memcpy(req_buf, &req_msg, req_fixed_len);

        tlv_iterator_begin(&req_iter, &req_buf[req_fixed_len], req_len - req_fixed_len);
        tlv_encode(&req_iter, &ciphertext_record);
        tlv_encode(&req_iter, &salt_record);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
                    TS_CRYPTO_OPCODE_ASYMMETRIC_DECRYPT, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                struct tlv_const_iterator resp_iter;
                struct tlv_record decoded_record;
                tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

                if (tlv_find_decode(&resp_iter, TS_CRYPTO_ASYMMETRIC_DECRYPT_OUT_TAG_PLAINTEXT, &decoded_record)) {

                    if (decoded_record.length <= output_size) {

                        memcpy(output, decoded_record.value, decoded_record.length);
                        *output_length = decoded_record.length;
                    }
                    else {
                        /* Provided buffer is too small */
                        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
                    }
                }
                else {
                    /* Mandatory response parameter missing */
                    psa_status = PSA_ERROR_GENERIC_ERROR;
                }
            }
        }

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}

psa_status_t packedc_crypto_client::generate_random(uint8_t *output, size_t output_size)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_generate_random_in req_msg;
    size_t req_len = sizeof(ts_crypto_generate_random_in);

    req_msg.size = output_size;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(m_caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;

        memcpy(req_buf, &req_msg, req_len);

        m_err_rpc_status = rpc_caller_invoke(m_caller, call_handle,
                TS_CRYPTO_OPCODE_GENERATE_RANDOM, &opstatus, &resp_buf, &resp_len);

        if (m_err_rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                struct tlv_const_iterator resp_iter;
                struct tlv_record decoded_record;
                tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

                if (tlv_find_decode(&resp_iter, TS_CRYPTO_GENERATE_RANDOM_OUT_TAG_RANDOM_BYTES, &decoded_record)) {

                    if (decoded_record.length <= output_size) {

                        memcpy(output, decoded_record.value, decoded_record.length);
                    }
                    else {
                        /* Provided buffer is too small */
                        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
                    }
                }
                else {
                    /* Mandatory response parameter missing */
                    psa_status = PSA_ERROR_GENERIC_ERROR;
                }
            }
        }

        rpc_caller_end(m_caller, call_handle);
    }

    return psa_status;
}

void packedc_crypto_client::translate_key_attributes(struct ts_crypto_key_attributes &proto_attributes,
                            const psa_key_attributes_t &psa_attributes)
{
    proto_attributes.type = psa_get_key_type(&psa_attributes);
    proto_attributes.key_bits = psa_get_key_bits(&psa_attributes);
    proto_attributes.lifetime = psa_get_key_lifetime(&psa_attributes);
    proto_attributes.id = psa_get_key_id(&psa_attributes);

    proto_attributes.policy.usage = psa_get_key_usage_flags(&psa_attributes);
    proto_attributes.policy.alg = psa_get_key_algorithm(&psa_attributes);
 }
