/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PSA_IPC_CRYPTO_CALLER_GENERATE_RANDOM_H
#define PSA_IPC_CRYPTO_CALLER_GENERATE_RANDOM_H

#include <string.h>
#include <stdlib.h>
#include <psa/crypto.h>
#include <psa/client.h>
#include <psa/sid.h>
#include <service/common/client/service_client.h>
#include <service/crypto/backend/psa_ipc/crypto_ipc_backend.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <protocols/service/crypto/packed-c/key_attributes.h>
#include <protocols/service/crypto/packed-c/import_key.h>
#include "crypto_caller_key_attributes.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline psa_status_t crypto_caller_generate_random(struct service_client *context,
							 uint8_t *output,
							 size_t output_size)
{
	struct service_client *ipc = context;
	struct rpc_caller *caller = ipc->caller;
	psa_status_t status;
	struct psa_ipc_crypto_pack_iovec iov = {
		.function_id = TFM_CRYPTO_GENERATE_RANDOM_SID,
	};
	struct psa_invec in_vec[] = {
		{ .base = psa_ptr_to_u32(&iov), .len = sizeof(struct psa_ipc_crypto_pack_iovec) },
	};
	struct psa_outvec out_vec[] = {
		{ .base = psa_ptr_to_u32(output), .len = output_size }
	};

	if (!output_size)
		return PSA_SUCCESS;

	status = psa_call(caller, TFM_CRYPTO_HANDLE, PSA_IPC_CALL, in_vec,
			  IOVEC_LEN(in_vec), out_vec, IOVEC_LEN(out_vec));

	return status;
}

#ifdef __cplusplus
}
#endif

#endif /* PSA_IPC_CRYPTO_CALLER_GENERATE_RANDOM_H */
