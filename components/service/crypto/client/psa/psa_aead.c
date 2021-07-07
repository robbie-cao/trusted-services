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

psa_status_t psa_aead_encrypt(psa_key_id_t key,
	psa_algorithm_t alg,
	const uint8_t *nonce,
	size_t nonce_length,
	const uint8_t *additional_data,
	size_t additional_data_length,
	const uint8_t *plaintext,
	size_t plaintext_length,
	uint8_t *ciphertext,
	size_t ciphertext_size,
	size_t *ciphertext_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_aead_decrypt(psa_key_id_t key,
	psa_algorithm_t alg,
	const uint8_t *nonce,
	size_t nonce_length,
	const uint8_t *additional_data,
	size_t additional_data_length,
	const uint8_t *ciphertext,
	size_t ciphertext_length,
	uint8_t *plaintext,
	size_t plaintext_size,
	size_t *plaintext_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}
