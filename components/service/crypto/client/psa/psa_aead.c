/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <psa/crypto.h>
#include "psa_crypto_client.h"
#include "crypto_caller_selector.h"


psa_status_t psa_aead_encrypt(psa_key_id_t key,
	psa_algorithm_t alg,
	const uint8_t *nonce,
	size_t nonce_length,
	const uint8_t *additional_data,
	size_t additional_data_length,
	const uint8_t *plaintext,
	size_t plaintext_length,
	uint8_t *aeadtext,
	size_t aeadtext_size,
	size_t *aeadtext_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_aead_decrypt(psa_key_id_t key,
	psa_algorithm_t alg,
	const uint8_t *nonce,
	size_t nonce_length,
	const uint8_t *additional_data,
	size_t additional_data_length,
	const uint8_t *aeadtext,
	size_t aeadtext_length,
	uint8_t *plaintext,
	size_t plaintext_size,
	size_t *plaintext_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_aead_encrypt_setup(psa_aead_operation_t *operation,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	if (psa_crypto_client_instance.init_status != PSA_SUCCESS)
		return psa_crypto_client_instance.init_status;

	return crypto_caller_aead_encrypt_setup(&psa_crypto_client_instance.base,
		&operation->handle, key, alg);
}

psa_status_t psa_aead_decrypt_setup(psa_aead_operation_t *operation,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	if (psa_crypto_client_instance.init_status != PSA_SUCCESS)
		return psa_crypto_client_instance.init_status;

	return crypto_caller_aead_decrypt_setup(&psa_crypto_client_instance.base,
		&operation->handle, key, alg);
}

psa_status_t psa_aead_generate_nonce(psa_aead_operation_t *operation,
	uint8_t *nonce,
	size_t nonce_size,
	size_t *nonce_length)
{
	return crypto_caller_aead_generate_nonce(&psa_crypto_client_instance.base,
		operation->handle,
		nonce, nonce_size, nonce_length);
}

psa_status_t psa_aead_set_nonce(psa_aead_operation_t *operation,
	const uint8_t *nonce,
	size_t nonce_length)
{
	return crypto_caller_aead_set_nonce(&psa_crypto_client_instance.base,
		operation->handle,
		nonce, nonce_length);
}

psa_status_t psa_aead_set_lengths(psa_aead_operation_t *operation,
	size_t ad_length,
	size_t plaintext_length)
{
	return crypto_caller_aead_set_lengths(&psa_crypto_client_instance.base,
		operation->handle,
		ad_length, plaintext_length);
}

psa_status_t psa_aead_update_ad(psa_aead_operation_t *operation,
	const uint8_t *input,
	size_t input_length)
{
	return crypto_caller_aead_update_ad(&psa_crypto_client_instance.base,
		operation->handle,
		input, input_length);
}

psa_status_t psa_aead_update(psa_aead_operation_t *operation,
	const uint8_t *input,
	size_t input_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	return crypto_caller_aead_update(&psa_crypto_client_instance.base,
		operation->handle,
		input, input_length,
		output, output_size, output_length);
}

psa_status_t psa_aead_finish(psa_aead_operation_t *operation,
	uint8_t *aeadtext,
	size_t aeadtext_size,
	size_t *aeadtext_length,
	uint8_t *tag,
	size_t tag_size,
	size_t *tag_length)
{
	return crypto_caller_aead_finish(&psa_crypto_client_instance.base,
		operation->handle,
		aeadtext, aeadtext_size, aeadtext_length,
		tag, tag_size, tag_length);
}

psa_status_t psa_aead_verify(psa_aead_operation_t *operation,
	uint8_t *plaintext,
	size_t plaintext_size,
	size_t *plaintext_length,
	const uint8_t *tag,
	size_t tag_length)
{
	return crypto_caller_aead_verify(&psa_crypto_client_instance.base,
		operation->handle,
		plaintext, plaintext_size, plaintext_length,
		tag, tag_length);
}

psa_status_t psa_aead_abort(psa_aead_operation_t *operation)
{
	return crypto_caller_aead_abort(&psa_crypto_client_instance.base,
		operation->handle);
}
