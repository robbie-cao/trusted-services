/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FFA_CALL_EP_H
#define FFA_CALL_EP_H

#include <ffa_api.h>
#include "sp_messaging.h"
#include <components/rpc/common/endpoint/rpc_interface.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NUM_MAX_SESS
#define NUM_MAX_SESS (16)
#endif

struct ffa_call_ep {
	struct rpc_interface *iface;
	unsigned long shmem_buf_handle[NUM_MAX_SESS];
	uint8_t *shmem_buf[NUM_MAX_SESS];
	size_t shmem_buf_size[NUM_MAX_SESS];
	uint16_t src_id[NUM_MAX_SESS];
 };

void ffa_call_ep_init(struct ffa_call_ep *ffa_call_ep, struct rpc_interface *iface);
void ffa_call_ep_receive(struct ffa_call_ep *call_ep,
			 const struct sp_msg *req_msg,
			 struct sp_msg *resp_msg);

#ifdef __cplusplus
}
#endif

#endif /* FFA_CALL_EP_H */
