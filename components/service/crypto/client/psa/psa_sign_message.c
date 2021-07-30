/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <psa/crypto.h>

psa_status_t psa_sign_message(
	psa_key_id_t key,
	psa_algorithm_t alg,
	const uint8_t *input,
	size_t input_length,
	uint8_t *signature,
	size_t signature_size,
	size_t *signature_length)
{
	size_t hash_len;
	uint8_t hash[PSA_HASH_MAX_SIZE];

	psa_status_t psa_status = psa_hash_compute(PSA_ALG_SIGN_GET_HASH(alg),
		input, input_length,
		hash, sizeof(hash), &hash_len);

	if (psa_status == PSA_SUCCESS) {

		psa_status = psa_sign_hash(key, alg,
			hash, hash_len,
			signature, signature_size, signature_length);
	}

	return psa_status;
}
