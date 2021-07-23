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
	return PSA_ERROR_NOT_SUPPORTED;
}
