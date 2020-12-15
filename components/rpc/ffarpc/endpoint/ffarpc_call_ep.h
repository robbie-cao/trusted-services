/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FFA_CALL_EP_H
#define FFA_CALL_EP_H

#include <ffa_api.h>
#include <components/rpc/common/endpoint/rpc_interface.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ffa_call_ep {
	struct rpc_interface *iface;
	unsigned long shmem_buf_handle;
	volatile uint8_t *shmem_buf;
	size_t shmem_buf_size;
 };

void ffa_call_ep_init(struct ffa_call_ep *ffa_call_ep, struct rpc_interface *iface);
void ffa_call_ep_receive(struct ffa_call_ep *call_ep,
			 const struct ffa_direct_msg *req_msg,
			 struct ffa_direct_msg *resp_msg);

#ifdef __cplusplus
}
#endif

#endif /* FFA_CALL_EP_H */
