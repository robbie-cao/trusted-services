/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWU_PROVIDER_SERIALIZER_H
#define FWU_PROVIDER_SERIALIZER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/uuid/uuid.h"
#include "rpc/common/endpoint/rpc_interface.h"

/* Provides a common interface for parameter serialization operations
 * for the fwu service provider. Allows alternative serialization
 * protocols to be used without hard-wiring a particular protocol
 * into the service provider code. A concrete serializer must
 * implement this interface.
 */
struct fwu_provider_serializer {
	/* Operation: open */
	rpc_status_t (*deserialize_open_req)(const struct call_param_buf *req_buf,
					     struct uuid_octets *image_type_uuid);

	rpc_status_t (*serialize_open_resp)(struct call_param_buf *resp_buf, uint32_t handle);

	/* Operation: write_stream */
	rpc_status_t (*deserialize_write_stream_req)(const struct call_param_buf *req_buf,
						     uint32_t *handle, size_t *data_len,
						     const uint8_t **data);

	/* Operation: read_stream */
	rpc_status_t (*deserialize_read_stream_req)(const struct call_param_buf *req_buf,
						    uint32_t *handle);

	void (*read_stream_resp_payload)(const struct call_param_buf *resp_buf,
					 uint8_t **payload_buf, size_t *max_payload);

	rpc_status_t (*serialize_read_stream_resp)(struct call_param_buf *resp_buf,
						   size_t read_bytes, size_t total_bytes);

	/* Operation: commit */
	rpc_status_t (*deserialize_commit_req)(const struct call_param_buf *req_buf,
					       uint32_t *handle, bool *accepted,
					       size_t *max_atomic_len);

	rpc_status_t (*serialize_commit_resp)(struct call_param_buf *resp_buf, size_t progress,
					      size_t total_work);

	/* Operation: accept_image */
	rpc_status_t (*deserialize_accept_req)(const struct call_param_buf *req_buf,
					       struct uuid_octets *image_type_uuid);
};

#endif /* FWU_PROVIDER_SERIALIZER_H */
