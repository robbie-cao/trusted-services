/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2023, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <rpc_caller.h>
#include <openamp_messenger_api.h>

struct psa_ipc_caller {
	struct rpc_caller rpc_caller;
	struct openamp_messenger openamp;
};

void *psa_ipc_phys_to_virt(void *context, void *pa);
void *psa_ipc_virt_to_phys(void *context, void *va);

struct rpc_caller *psa_ipc_caller_init(struct psa_ipc_caller *psaipc);
struct rpc_caller *psa_ipc_caller_deinit(struct psa_ipc_caller *psaipc);
