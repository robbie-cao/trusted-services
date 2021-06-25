/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPC_INTERFACE_H
#define RPC_INTERFACE_H

#include <stddef.h>
#include <stdint.h>
#include <rpc_status.h>
#include <protocols/rpc/common/packed-c/status.h>

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

/** \brief Call request
 *
 * A call request object represents a request from a client that will
 * be handled by a call endpoint.
 */
struct call_req {
	uint32_t caller_id;
	uint32_t interface_id;
	uint32_t opcode;
	uint32_t encoding;
	int opstatus;
	struct call_param_buf req_buf;
	struct call_param_buf resp_buf;
};

static inline uint32_t call_req_get_caller_id(const struct call_req *req)
{
	return req->caller_id;
}

static inline uint32_t call_req_get_interface_id(const struct call_req *req)
{
	return req->interface_id;
}

static inline uint32_t call_req_get_opcode(const struct call_req *req)
{
	return req->opcode;
}

static inline uint32_t call_req_get_encoding(const struct call_req *req)
{
	return req->encoding;
}

static inline int call_req_get_opstatus(const struct call_req *req)
{
	return req->opstatus;
}

static inline void call_req_set_opstatus(struct call_req *req, int opstatus)
{
	req->opstatus = opstatus;
}

static inline struct call_param_buf *call_req_get_req_buf(struct call_req *req)
{
	return &req->req_buf;
}

static inline struct call_param_buf *call_req_get_resp_buf(struct call_req *req)
{
	return &req->resp_buf;
}

/** \brief RPC interface
 *
 * A generalized RPC interface.  Provides a standard interface for a
 * call endpoint that handles incoming call requests.
 */
struct rpc_interface
{
	void *context;
	rpc_status_t (*receive)(struct rpc_interface *iface, struct call_req *req);
};

static inline rpc_status_t rpc_interface_receive(struct rpc_interface *iface,
					  struct call_req *req)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERFACE_DOES_NOT_EXIST;

	if (iface) {

		rpc_status = iface->receive(iface, req);
	}

	return rpc_status;
}

#ifdef __cplusplus
}
#endif

#endif /* RPC_INTERFACE_H */
