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

psa_status_t psa_raw_key_agreement(psa_algorithm_t alg,
	psa_key_id_t private_key,
	const uint8_t *peer_key,
	size_t peer_key_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}
