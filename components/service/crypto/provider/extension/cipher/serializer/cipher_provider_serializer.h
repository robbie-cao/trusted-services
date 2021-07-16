/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CIPHER_PROVIDER_SERIALIZER_H
#define CIPHER_PROVIDER_SERIALIZER_H

#include <stddef.h>
#include <stdint.h>
#include <psa/crypto.h>
#include <rpc/common/endpoint/rpc_interface.h>

/* Provides a common interface for parameter serialization operations
 * for the cipher service provider.
 */
struct cipher_provider_serializer {

	/* Operation: cipher_setup */
	rpc_status_t (*deserialize_cipher_setup_req)(const struct call_param_buf *req_buf,
		psa_key_id_t *id,
		psa_algorithm_t *alg);

	rpc_status_t (*serialize_cipher_setup_resp)(struct call_param_buf *resp_buf,
		uint32_t op_handle);

	/* Operation: cipher_generate_iv */
	rpc_status_t (*deserialize_cipher_generate_iv_req)(const struct call_param_buf *req_buf,
		uint32_t *op_handle);

	rpc_status_t (*serialize_cipher_generate_iv_resp)(struct call_param_buf *resp_buf,
		const uint8_t *iv, size_t iv_len);

	/* Operation: cipher_set_iv */
	rpc_status_t (*deserialize_cipher_set_iv_req)(const struct call_param_buf *req_buf,
		uint32_t *op_handle,
		const uint8_t **iv, size_t *iv_len);

	/* Operation: cipher_update */
	rpc_status_t (*deserialize_cipher_update_req)(const struct call_param_buf *req_buf,
		uint32_t *op_handle,
		const uint8_t **data, size_t *data_len);

	rpc_status_t (*serialize_cipher_update_resp)(struct call_param_buf *resp_buf,
		const uint8_t *data, size_t data_len);

	/* Operation: cipher_finish */
	rpc_status_t (*deserialize_cipher_finish_req)(const struct call_param_buf *req_buf,
		uint32_t *op_handle);

	rpc_status_t (*serialize_cipher_finish_resp)(struct call_param_buf *resp_buf,
		const uint8_t *data, size_t data_len);

	/* Operation: cipher_abort */
	rpc_status_t (*deserialize_cipher_abort_req)(const struct call_param_buf *req_buf,
		uint32_t *op_handle);
};

#endif /* CIPHER_PROVIDER_SERIALIZER_H */
