/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2020-2021, Arm Limited
 */

#ifndef FFA_CALL_EP_H
#define FFA_CALL_EP_H

#include <components/rpc/common/endpoint/rpc_interface.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NUM_MAX_SESS
#define NUM_MAX_SESS (16)
#endif

struct thread_smc_args;

struct ffa_call_ep {
	struct rpc_interface *iface;
	unsigned long shmem_buf_handle[NUM_MAX_SESS];
	uint8_t *shmem_buf[NUM_MAX_SESS];
	size_t shmem_buf_size[NUM_MAX_SESS];
	uint16_t src_id[NUM_MAX_SESS];
};

void ffa_call_ep_init(struct ffa_call_ep *ffa_call_ep,
		      struct rpc_interface *iface);
void ffa_call_ep_receive(struct ffa_call_ep *call_ep,
			 const struct thread_smc_args *req_msg,
			 struct thread_smc_args *resp_msg);
void *ffa_call_ep_get_buffer(struct ffa_call_ep *call_ep, uint16_t source_id,
			     size_t *buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* FFA_CALL_EP_H */
