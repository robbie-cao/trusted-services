/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <rpc/mm_communicate/caller/linux/mm_communicate_caller.h>
#include "mm_communicate_service_context.h"

/* Concrete service_context methods */
static rpc_session_handle mm_communicate_service_context_open(void *context,
	struct rpc_caller **caller);
static void mm_communicate_service_context_close(void *context,
	rpc_session_handle session_handle);
static void mm_communicate_service_context_relinquish(void *context);


struct mm_communicate_service_context *mm_communicate_service_context_create(
	const char *dev_path,
	uint16_t partition_id,
	const EFI_GUID *svc_guid)
{
	struct mm_communicate_service_context *new_context = (struct mm_communicate_service_context*)
		malloc(sizeof(struct mm_communicate_service_context));

	if (new_context) {

		new_context->ffa_dev_path = dev_path;
		new_context->partition_id = partition_id;
		new_context->svc_guid = *svc_guid;

		new_context->service_context.context = new_context;
		new_context->service_context.open = mm_communicate_service_context_open;
		new_context->service_context.close = mm_communicate_service_context_close;
		new_context->service_context.relinquish = mm_communicate_service_context_relinquish;
	}

	return new_context;
}

static rpc_session_handle mm_communicate_service_context_open(
	void *context,
	struct rpc_caller **caller)
{
	struct mm_communicate_service_context *this_context =
		(struct mm_communicate_service_context*)context;

	rpc_session_handle session_handle = NULL;

	if (!mm_communicate_caller_check_version())
		return NULL;

	struct mm_communicate_caller *mm_communicate_caller =
		(struct mm_communicate_caller*)malloc(sizeof(struct mm_communicate_caller));

	if (mm_communicate_caller) {

		*caller = mm_communicate_caller_init(mm_communicate_caller,
			this_context->ffa_dev_path);
		int status = mm_communicate_caller_open(mm_communicate_caller,
			this_context->partition_id, &this_context->svc_guid);

		if (status == 0) {
			/* Successfully opened session */
			session_handle = mm_communicate_caller;
		}
		else {
			/* Failed to open session */
			mm_communicate_caller_close(mm_communicate_caller);
			mm_communicate_caller_deinit(mm_communicate_caller);
			free(mm_communicate_caller);
		}
	}

	return session_handle;
}

static void mm_communicate_service_context_close(
	void *context,
	rpc_session_handle session_handle)
{
	struct mm_communicate_caller *mm_communicate_caller =
		(struct mm_communicate_caller*)session_handle;

	(void)context;

	if (mm_communicate_caller) {

		mm_communicate_caller_close(mm_communicate_caller);
		mm_communicate_caller_deinit(mm_communicate_caller);
		free(mm_communicate_caller);
	}
}

static void mm_communicate_service_context_relinquish(
	void *context)
{
	struct mm_communicate_service_context *this_context =
		(struct mm_communicate_service_context*)context;

	free(this_context);
}
