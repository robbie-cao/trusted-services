/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <psa/crypto.h>
#include "psa_crypto_client.h"
#include "crypto_caller_selector.h"


psa_status_t psa_cipher_encrypt_setup(psa_cipher_operation_t *operation,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	return crypto_caller_cipher_encrypt_setup(&psa_crypto_client_instance.base,
		&operation->handle,
		key, alg);
}

psa_status_t psa_cipher_decrypt_setup(psa_cipher_operation_t *operation,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	return crypto_caller_cipher_decrypt_setup(&psa_crypto_client_instance.base,
		&operation->handle,
		key, alg);
}

psa_status_t psa_cipher_generate_iv(psa_cipher_operation_t *operation,
	uint8_t *iv,
	size_t iv_size,
	size_t *iv_length)
{
	return crypto_caller_cipher_generate_iv(&psa_crypto_client_instance.base,
		operation->handle,
		iv, iv_size, iv_length);
}

psa_status_t psa_cipher_set_iv(psa_cipher_operation_t *operation,
	const uint8_t *iv,
	size_t iv_length)
{
	return crypto_caller_cipher_set_iv(&psa_crypto_client_instance.base,
		operation->handle,
		iv, iv_length);
}

psa_status_t psa_cipher_update(psa_cipher_operation_t *operation,
	const uint8_t *input,
	size_t input_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	return crypto_caller_cipher_update(&psa_crypto_client_instance.base,
		operation->handle,
		input, input_length,
		output, output_size, output_length);
}

psa_status_t psa_cipher_finish(psa_cipher_operation_t *operation,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	return crypto_caller_cipher_finish(&psa_crypto_client_instance.base,
		operation->handle,
		output, output_size, output_length);
}

psa_status_t psa_cipher_abort(psa_cipher_operation_t *operation)
{
	return crypto_caller_cipher_abort(&psa_crypto_client_instance.base,
		operation->handle);
}

static psa_status_t multi_cipher_update(psa_cipher_operation_t *operation,
	const uint8_t *input,
	size_t input_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	psa_status_t psa_status = PSA_SUCCESS;
	size_t max_update_size =
		crypto_caller_cipher_max_update_size(&psa_crypto_client_instance.base);
	size_t bytes_input = 0;
	size_t bytes_output = 0;

	*output_length = 0;

	if (!max_update_size) {

		/* Don't know the max update size so assume that the entire
		 * input and output can be handled in a single update.  If
		 * this isn't true, the first cipher update operation will fail
		 * safely.
		 */
		max_update_size = input_length;
	}

	while ((bytes_input < input_length) && (bytes_output < output_size)) {

		size_t update_output_len = 0;
		size_t bytes_remaining = input_length - bytes_input;
		size_t update_len = (bytes_remaining < max_update_size) ?
			bytes_remaining :
			max_update_size;

		psa_status = psa_cipher_update(operation,
			&input[bytes_input], update_len,
			&output[bytes_output], output_size - bytes_output, &update_output_len);

		if (psa_status != PSA_SUCCESS) {

			psa_cipher_abort(operation);
			break;
		}

		bytes_input += update_len;
		bytes_output += update_output_len;
	}

	if (psa_status == PSA_SUCCESS) {

		if (bytes_output < output_size) {

			size_t finish_output_len = 0;

			psa_status = psa_cipher_finish(operation,
				&output[bytes_output], output_size - bytes_output, &finish_output_len);

			if (psa_status == PSA_SUCCESS) {

				*output_length = bytes_output + finish_output_len;
			}
		}
		else {

			psa_cipher_abort(operation);
			psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
		}
	}

	return psa_status;
}

psa_status_t psa_cipher_encrypt(psa_key_id_t key,
	psa_algorithm_t alg,
	const uint8_t *input,
	size_t input_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	psa_cipher_operation_t operation = psa_cipher_operation_init();
	psa_status_t psa_status = psa_cipher_encrypt_setup(&operation, key, alg);

	if (psa_status == PSA_SUCCESS) {

		psa_status = multi_cipher_update(&operation,
			input, input_length,
			output, output_size, output_length);
	}

	return psa_status;
}

psa_status_t psa_cipher_decrypt(psa_key_id_t key,
	psa_algorithm_t alg,
	const uint8_t *input,
	size_t input_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	psa_cipher_operation_t operation = psa_cipher_operation_init();
	psa_status_t psa_status = psa_cipher_decrypt_setup(&operation, key, alg);

	if (psa_status == PSA_SUCCESS) {

		psa_status = multi_cipher_update(&operation,
			input, input_length,
			output, output_size, output_length);
	}

	return psa_status;
}
