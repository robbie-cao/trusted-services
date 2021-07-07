/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <psa/crypto.h>
#include "psa_crypto_client.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>


psa_status_t psa_copy_key(psa_key_id_t source_key,
	const psa_key_attributes_t *attributes,
	psa_key_id_t *target_key)
{
	return PSA_ERROR_NOT_SUPPORTED;
}
