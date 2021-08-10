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
	struct service_info get_service_info() const;

	/* Key lifecycle methods */
	virtual psa_status_t generate_key(
		const psa_key_attributes_t *attributes,
		psa_key_id_t *id) = 0;

	virtual psa_status_t destroy_key(
		psa_key_id_t id) = 0;

	virtual psa_status_t import_key(
		const psa_key_attributes_t *attributes,
		const uint8_t *data, size_t data_length,
		psa_key_id_t *id) = 0;

	virtual psa_status_t copy_key(
		psa_key_id_t source_key,
		const psa_key_attributes_t *attributes,
		psa_key_id_t *target_key) = 0;

	virtual psa_status_t purge_key(
		psa_key_id_t id) = 0;

	virtual psa_status_t get_key_attributes(
		psa_key_id_t id,
		psa_key_attributes_t *attributes) = 0;

	/* Key export methods */
	virtual psa_status_t export_key(
		psa_key_id_t id,
		uint8_t *data, size_t data_size, size_t *data_length) = 0;

	virtual psa_status_t export_public_key(
		psa_key_id_t id,
		uint8_t *data, size_t data_size, size_t *data_length) = 0;

	/* Sign/verify methods */
	virtual psa_status_t sign_hash(
		psa_key_id_t id,
		psa_algorithm_t alg,
		const uint8_t *hash, size_t hash_length,
		uint8_t *signature, size_t signature_size, size_t *signature_length) = 0;

	virtual psa_status_t verify_hash(
		psa_key_id_t id,
		psa_algorithm_t alg,
		const uint8_t *hash, size_t hash_length,
		const uint8_t *signature, size_t signature_length) = 0;

	/* Asymmetric encrypt/decrypt */
	virtual psa_status_t asymmetric_encrypt(
		psa_key_id_t id,
		psa_algorithm_t alg,
		const uint8_t *input, size_t input_length,
		const uint8_t *salt, size_t salt_length,
		uint8_t *output, size_t output_size, size_t *output_length) = 0;

	virtual psa_status_t asymmetric_decrypt(
		psa_key_id_t id,
		psa_algorithm_t alg,
		const uint8_t *input, size_t input_length,
		const uint8_t *salt, size_t salt_length,
		uint8_t *output, size_t output_size, size_t *output_length) = 0;

	/* Random number generation */
	virtual psa_status_t generate_random(
		uint8_t *output, size_t output_size) = 0;

	/* Hash methods */
	virtual size_t hash_max_update_size() const = 0;

	virtual psa_status_t hash_setup(
		uint32_t *op_handle,
		psa_algorithm_t alg) = 0;

	virtual psa_status_t hash_update(
		uint32_t op_handle,
		const uint8_t *input, size_t input_length) = 0;

	virtual psa_status_t hash_finish(
		uint32_t op_handle,
		uint8_t *hash, size_t hash_size, size_t *hash_length) = 0;

	virtual psa_status_t hash_abort(
		uint32_t op_handle) = 0;

	virtual psa_status_t hash_verify(
		uint32_t op_handle,
		const uint8_t *hash, size_t hash_length) = 0;

	virtual psa_status_t hash_clone(
		uint32_t source_op_handle,
		uint32_t *target_op_handle) = 0;

	/* Cipher methods */
	virtual size_t cipher_max_update_size() const = 0;

	virtual psa_status_t cipher_encrypt_setup(
		uint32_t *op_handle,
		psa_key_id_t key,
		psa_algorithm_t alg) = 0;

	virtual psa_status_t cipher_decrypt_setup(
		uint32_t *op_handle,
		psa_key_id_t key,
		psa_algorithm_t alg) = 0;

	virtual psa_status_t cipher_generate_iv(
		uint32_t op_handle,
		uint8_t *iv, size_t iv_size, size_t *iv_length) = 0;

	virtual psa_status_t cipher_set_iv(
		uint32_t op_handle,
		const uint8_t *iv, size_t iv_length) = 0;

	virtual psa_status_t cipher_update(
		uint32_t op_handle,
		const uint8_t *input, size_t input_length,
		uint8_t *output, size_t output_size, size_t *output_length) = 0;

	virtual psa_status_t cipher_finish(
		uint32_t op_handle,
		uint8_t *output, size_t output_size, size_t *output_length) = 0;

	virtual psa_status_t cipher_abort(
		uint32_t op_handle) = 0;

	/* MAC methods */
	virtual size_t mac_max_update_size() const = 0;

	virtual psa_status_t mac_sign_setup(
		uint32_t *op_handle,
		psa_key_id_t key,
		psa_algorithm_t alg) = 0;

	virtual psa_status_t mac_verify_setup(
		uint32_t *op_handle,
		psa_key_id_t key,
		psa_algorithm_t alg) = 0;

	virtual psa_status_t mac_update(
		uint32_t op_handle,
		const uint8_t *input, size_t input_length) = 0;

	virtual psa_status_t mac_sign_finish(
		uint32_t op_handle,
		uint8_t *mac, size_t mac_size, size_t *mac_length) = 0;

	virtual psa_status_t mac_verify_finish(
		uint32_t op_handle,
		const uint8_t *mac, size_t mac_length) = 0;

	virtual psa_status_t mac_abort(
		uint32_t op_handle) = 0;


protected:
	crypto_client();
	crypto_client(struct rpc_caller *caller);
	void set_caller(struct rpc_caller *caller);

	struct service_client m_client;
};

#endif /* CRYPTO_CLIENT_H */
