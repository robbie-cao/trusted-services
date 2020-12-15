/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FFARPC_CALLER_H
#define FFARPC_CALLER_H

#include <common/uuid/uuid.h>
#include <rpc_caller.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * An RPC caller for Linux user-space clients.  Uses the FFA kernel driver
 * to communicate with RPC endpoints deployed in partitions accessible
 * via FFA.
 */
struct ffarpc_caller {
	struct rpc_caller rpc_caller;
	int fd;
	const char *device_path;
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

struct rpc_caller *ffarpc_caller_init(struct ffarpc_caller *s, const char *device_path);
void ffarpc_caller_deinit(struct ffarpc_caller *s);
size_t ffarpc_caller_discover(const struct ffarpc_caller *s, const struct uuid_canonical *uuid,
						uint16_t *partition_ids, size_t discover_limit);
int ffarpc_caller_open(struct ffarpc_caller *s, uint16_t dest_partition_id, uint16_t dest_iface_id);
int ffarpc_caller_close(struct ffarpc_caller *s);

#ifdef __cplusplus
}
#endif

#endif /* FFARPC_CALLER_H */
