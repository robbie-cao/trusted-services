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
#include <protocols/service/crypto/packed-c/hash.h>
#include <common/tlv/tlv.h>

psa_status_t psa_hash_setup(psa_hash_operation_t *operation,
	psa_algorithm_t alg)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_hash_update(psa_hash_operation_t *operation,
	const uint8_t *input,
	size_t input_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_hash_finish(psa_hash_operation_t *operation,
	uint8_t *hash,
	size_t hash_size,
	size_t *hash_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_hash_abort(psa_hash_operation_t *operation)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_hash_verify(psa_hash_operation_t *operation,
	const uint8_t *hash,
	size_t hash_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_hash_clone(const psa_hash_operation_t *source_operation,
	psa_hash_operation_t *target_operation)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_hash_compare(psa_algorithm_t alg,
	const uint8_t *input,
	size_t input_length,
	const uint8_t *hash,
	size_t hash_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_hash_compute(psa_algorithm_t alg,
	const uint8_t *input,
	size_t input_length,
	uint8_t *hash,
	size_t hash_size,
	size_t *hash_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}
