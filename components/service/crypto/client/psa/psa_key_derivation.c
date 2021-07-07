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

psa_status_t psa_key_derivation_setup(
	psa_key_derivation_operation_t *operation,
	psa_algorithm_t alg)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_get_capacity(
	const psa_key_derivation_operation_t *operation,
	size_t *capacity)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_set_capacity(
	psa_key_derivation_operation_t *operation,
	size_t capacity)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_input_bytes(
	psa_key_derivation_operation_t *operation,
	psa_key_derivation_step_t step,
	const uint8_t *data,
	size_t data_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_input_key(
	psa_key_derivation_operation_t *operation,
	psa_key_derivation_step_t step,
	psa_key_id_t key)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_key_agreement(
	psa_key_derivation_operation_t *operation,
	psa_key_derivation_step_t step,
	psa_key_id_t private_key,
	const uint8_t *peer_key,
	size_t peer_key_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_output_bytes(
	psa_key_derivation_operation_t *operation,
	uint8_t *output,
	size_t output_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_output_key(
	const psa_key_attributes_t *attributes,
	psa_key_derivation_operation_t *operation,
	psa_key_id_t *key)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_abort(
	psa_key_derivation_operation_t *operation)
{
	return PSA_ERROR_NOT_SUPPORTED;
}
