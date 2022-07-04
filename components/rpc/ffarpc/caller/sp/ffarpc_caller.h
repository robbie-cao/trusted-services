/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FFA_SERVICE_CALLER_H
#define FFA_SERVICE_CALLER_H

#include <rpc_caller.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ffarpc_caller {
	struct rpc_caller rpc_caller;
	uint16_t own_id;
	uint16_t dest_partition_id;
	uint16_t dest_iface_id;
	uint64_t shared_mem_handle;
	size_t shared_mem_required_size;
	uint8_t *req_buf;
	uint8_t *resp_buf;
	size_t req_len;
	size_t resp_len;
	bool is_call_transaction_in_progess;
};

struct rpc_caller *ffarpc_caller_init(struct ffarpc_caller *s, uint16_t own_id);
void ffarpc_caller_deinit(struct ffarpc_caller *s);
uint32_t ffarpc_caller_discover(const uint8_t *uuid, uint16_t *sp_ids, uint32_t sp_max_cnt);
int ffarpc_caller_open(struct ffarpc_caller *s, uint16_t dest_partition_id, uint16_t dest_iface_id);
int ffarpc_caller_close(struct ffarpc_caller *s);

#ifdef __cplusplus
}
#endif

#endif /* FFA_SERVICE_CALLER_H */
