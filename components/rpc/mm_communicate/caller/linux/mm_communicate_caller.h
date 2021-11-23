/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MM_COMMUNICATE_CALLER_H
#define MM_COMMUNICATE_CALLER_H

#include <stdbool.h>
#include <common/uuid/uuid.h>
#include <protocols/common/efi/efi_types.h>
#include <rpc_caller.h>
#include "mm_communicate_serializer.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
 * An RPC caller for Linux user-space clients that uses the MM_COMMUNICATE
 * protocol for calling UEFI SMM service endpoints.
 */
struct mm_communicate_caller
{
	struct rpc_caller rpc_caller;
	int ffa_fd;
	const char *ffa_device_path;
	uint16_t dest_partition_id;
	uint8_t *comm_buffer;
	size_t comm_buffer_size;
	size_t req_len;
	size_t scrub_len;
	bool is_call_transaction_in_progess;
	const struct mm_communicate_serializer *serializer;
};

bool mm_communicate_caller_check_version(void);

struct rpc_caller *mm_communicate_caller_init(
	struct mm_communicate_caller *s,
	const char *ffa_device_path);

void mm_communicate_caller_deinit(
	struct mm_communicate_caller *s);

size_t mm_communicate_caller_discover(
	const struct mm_communicate_caller *s,
	const struct uuid_canonical *uuid,
	uint16_t *partition_ids,
	size_t discover_limit);

int mm_communicate_caller_open(
	struct mm_communicate_caller *s,
	uint16_t dest_partition_id,
	const EFI_GUID *svc_guid);

int mm_communicate_caller_close(
	struct mm_communicate_caller *s);

#ifdef __cplusplus
}
#endif

#endif /* MM_COMMUNICATE_CALLER_H */
