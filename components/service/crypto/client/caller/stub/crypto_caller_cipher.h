/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STUB_CRYPTO_CALLER_CIPHER_H
#define STUB_CRYPTO_CALLER_CIPHER_H

#include <psa/crypto.h>
#include <service/common/client/service_client.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline psa_status_t crypto_caller_cipher_encrypt_setup(struct service_client *context,
	uint32_t *op_handle,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

static inline psa_status_t crypto_caller_cipher_decrypt_setup(struct service_client *context,
	uint32_t *op_handle,
	psa_key_id_t key,
	psa_algorithm_t alg)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

static inline psa_status_t crypto_caller_cipher_generate_iv(struct service_client *context,
	uint32_t op_handle,
	uint8_t *iv,
	size_t iv_size,
	size_t *iv_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

static inline psa_status_t crypto_caller_cipher_set_iv(struct service_client *context,
	uint32_t op_handle,
	const uint8_t *iv,
	size_t iv_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

static inline psa_status_t crypto_caller_cipher_update(struct service_client *context,
	uint32_t op_handle,
	const uint8_t *input,
	size_t input_length,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

static inline psa_status_t crypto_caller_cipher_finish(struct service_client *context,
	uint32_t op_handle,
	uint8_t *output,
	size_t output_size,
	size_t *output_length)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

static inline psa_status_t crypto_caller_cipher_abort(struct service_client *context,
	uint32_t op_handle)
{
	return PSA_ERROR_NOT_SUPPORTED;
}

static inline size_t crypto_caller_cipher_max_update_size(struct service_client *context)
{
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* STUB_CRYPTO_CALLER_CIPHER_H */
