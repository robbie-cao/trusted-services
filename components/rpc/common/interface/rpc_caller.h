/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPC_CALLER_H
#define RPC_CALLER_H

#include <stddef.h>
#include <stdint.h>
#include "rpc_status.h"

/*
 * The rpc_caller puplic interface may be exported as a public interface to
 * a shared library.
 */
#ifdef EXPORT_PUBLIC_INTERFACE_RPC_CALLER
#define RPC_CALLER_EXPORTED __attribute__((__visibility__("default")))
#else
#define RPC_CALLER_EXPORTED
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Defines an abstract interface for calling operations provided by an rpc endpoint.
 * Concrete specializations will map the an RPC or direct calling mechanism to
 * suite the deployment.
 */

typedef void *rpc_call_handle;

struct rpc_caller
{
	void *context;
	uint32_t encoding;

	/* A concrete rpc_caller implements these methods */
	rpc_call_handle (*call_begin)(void *context, uint8_t **req_buf, size_t req_len);

	rpc_status_t (*call_invoke)(void *context, rpc_call_handle handle, uint32_t opcode,
		     	rpc_opstatus_t *opstatus, uint8_t **resp_buf, size_t *resp_len);

	void (*call_end)(void *context, rpc_call_handle handle);
};

/*
 * Called by a concrete rpc_caller to initialise the base rpc_caller.
 */
void rpc_caller_init(struct rpc_caller *s, void *context);

/*
 * Allows a client to specify the parameter encoding scheme that the client
 * intends to use during an RPC session.  It is the client's responsiblity
 * to choose an encoding scheme that is supported by the remote interface.
 */
RPC_CALLER_EXPORTED void rpc_caller_set_encoding_scheme(struct rpc_caller *s,
			uint32_t encoding);

/*
 * Starts a call transaction. The returned handle is an identifier for the
 * transaction and must be passed as a parameter to call_invoke() and
 * call_end(). A concrete rpc_caller may perform resource allocation during
 * this call. This will include a buffer for the request message parameters.
 * Returns a NULL handle on failure.
 */
RPC_CALLER_EXPORTED rpc_call_handle rpc_caller_begin(struct rpc_caller *s,
			uint8_t **req_buf, size_t req_len);

/*
 * Invokes the operation identified by the opcode. This method blocks
 * until the operation completes. The status of the call is returned. An
 * additional endpoint specific status value is also returned. If a response
 * message was received, the concrete rpc_caller will have allocated a
 * buffer for the reponse. This buffer will hold valid data until the point when
 * call_end() is called for the transaction.
 */
RPC_CALLER_EXPORTED rpc_status_t rpc_caller_invoke(struct rpc_caller *s, rpc_call_handle handle,
			uint32_t opcode, rpc_opstatus_t *opstatus, uint8_t **resp_buf, size_t *resp_len);

/*
 * Ends the call transaction, allowing any resource associated with the
 * transaction to be freed.
 */
RPC_CALLER_EXPORTED void rpc_caller_end(struct rpc_caller *s, rpc_call_handle handle);

#ifdef __cplusplus
}
#endif

#endif /* RPC_CALLER_H */
