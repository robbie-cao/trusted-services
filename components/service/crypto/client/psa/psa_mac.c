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

psa_status_t psa_mac_sign_setup(psa_mac_operation_t *operation,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_mac_verify_setup(psa_mac_operation_t *operation,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_mac_update(psa_mac_operation_t *operation,
	const uint8_t *input,
	size_t input_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_mac_sign_finish(psa_mac_operation_t *operation,
	uint8_t *mac,
	size_t mac_size,
	size_t *mac_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_mac_verify_finish(psa_mac_operation_t *operation,
	const uint8_t *mac,
	size_t mac_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_mac_abort(psa_mac_operation_t *operation)
{
	return PSA_ERROR_NOT_SUPPORTED;
}
