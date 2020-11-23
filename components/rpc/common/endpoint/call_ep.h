/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CALL_EP_H
#define CALL_EP_H

#include <stddef.h>
#include <stdint.h>
#include <rpc_status.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions related to an rpc call endpoint */

/** \brief Call parameter buffer
 *
 * Describes a buffer for holding call request and response parameters.
 */
struct call_param_buf {
	size_t size;
	size_t data_len;
	void *data;
};

static inline struct call_param_buf call_param_buf_init_empty(void *data, size_t size)
{
	struct call_param_buf v;

	v.size = size;
	v.data_len = 0;
	v.data = data;

	return v;
}

static inline struct call_param_buf call_param_buf_init_full(void *data,
							       size_t size,
							       size_t data_len)
{
	struct call_param_buf v;

	v.size = size;
	v.data_len = data_len;
	v.data = data;

	return v;
}

/** \brief Serializer for handling call parameters
 *
 * An abstract serializer pointer, used for attaching a concrete
 * serializer to a call request for serializing/deserializing call
 * parameters.  The strategy for selecting an appropriate serializer
 * could be hard-coded or dynamic, based on a content type identifier
 * carried by a concrete rpc.
 */
typedef const void* call_param_serializer_ptr;

/** \brief Call request
 *
 * A call request object represents a request from a client that will
 * be handled by a call endpoint.
 */
struct call_req {
	uint32_t caller_id;
	uint32_t opcode;
	int opstatus;
	call_param_serializer_ptr serializer;
	struct call_param_buf req_buf;
	struct call_param_buf resp_buf;
};

static inline uint32_t call_req_get_caller_id(const struct call_req *req)
{
	return req->caller_id;
}

static inline uint32_t call_req_get_opcode(const struct call_req *req)
{
	return req->opcode;
}

static inline int call_req_get_opstatus(const struct call_req *req)
{
	return req->opstatus;
}

static inline void call_req_set_opstatus(struct call_req *req, int opstatus)
{
	req->opstatus = opstatus;
}

static inline call_param_serializer_ptr call_req_get_serializer(const struct call_req *req)
{
    return req->serializer;
}

static inline struct call_param_buf *call_req_get_req_buf(struct call_req *req)
{
	return &req->req_buf;
}

static inline struct call_param_buf *call_req_get_resp_buf(struct call_req *req)
{
	return &req->resp_buf;
}

/** \brief Call endpoint
 *
 * A generalized call endpoint.  Provides a standard interface for a
 * call endpoint that handles incoming call requests.
 */
struct call_ep
{
	void *context;
	rpc_status_t (*receive)(struct call_ep *ep, struct call_req *req);
};

static inline rpc_status_t call_ep_receive(struct call_ep *ep,
					  struct call_req *req)
{
	return ep->receive(ep, req);
}

#ifdef __cplusplus
}
#endif

#endif /* CALL_EP_H */
