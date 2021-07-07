/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <psa/crypto.h>
#include "psa_crypto_client.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <common/tlv/tlv.h>

psa_status_t psa_cipher_encrypt_setup(psa_cipher_operation_t *operation,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_cipher_decrypt_setup(psa_cipher_operation_t *operation,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_cipher_generate_iv(psa_cipher_operation_t *operation,
	uint8_t *iv,
	size_t iv_size,
	size_t *iv_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_cipher_set_iv(psa_cipher_operation_t *operation,
	const uint8_t *iv,
	size_t iv_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_cipher_update(psa_cipher_operation_t *operation,
	const uint8_t *input,
	size_t input_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_cipher_finish(psa_cipher_operation_t *operation,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_cipher_abort(psa_cipher_operation_t *operation)
{
	return PSA_ERROR_NOT_SUPPORTED;
}
