// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include "protocols/rpc/common/packed-c/status.h"
#include "rpc/ffarpc/caller/sp/ffarpc_caller.h"
#include "service/log/client/log_client.h"
#include <stdbool.h>
#include <stddef.h>

#define OPTEE_LOG_UUID_BYTES                                                                       \
	{                                                                                          \
		0xda, 0x9d, 0xff, 0xbd, 0xd5, 0x90, 0x40, 0xed, 0x97, 0x5f, 0x19, 0xc6, 0x5a,      \
			0x3d, 0x52, 0xd3                                                           \
	}

static const uint8_t optee_log_uuid[] = OPTEE_LOG_UUID_BYTES;

struct log_backend {
	struct ffarpc_caller ffarpc_caller;
	bool in_use;
};

static struct log_backend backend_instance = { .in_use = false };

struct log_backend *log_factory_create(void)
{
	struct rpc_caller *log_caller = NULL;
	uint16_t log_sp_id = 0;
	struct log_backend *new_backend = &backend_instance;

	if (new_backend->in_use)
		return new_backend;

	log_caller = ffarpc_caller_init(&new_backend->ffarpc_caller);

	if (ffarpc_caller_discover(optee_log_uuid, &log_sp_id, 1))
		if (ffarpc_caller_open(&new_backend->ffarpc_caller, log_sp_id, 0) == 0) {
			log_client_init(log_caller);
			new_backend->in_use = true;
		}

	/* Failed to discover or open an RPC session with provider */
	if (!new_backend->in_use) {
		ffarpc_caller_deinit(&new_backend->ffarpc_caller);
		return NULL;
	}

	return new_backend;
}

void log_factory_destroy(struct log_backend *backend)
{
	if (backend) {
		ffarpc_caller_deinit(&backend_instance.ffarpc_caller);
		backend_instance.in_use = false;
	}
}
