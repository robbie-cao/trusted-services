// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (C) 2021-2022, Arm Limited
 */

/*
 * This pseudo SP implements the storage backend using OP-TEE's storage system.
 */

#include <string.h>

#include <ffa.h>
#include <kernel/pseudo_sp.h>
#include "components/rpc/ffarpc/endpoint/sel1_sp/sel1_sp_ffarpc_call_ep.h"
#include "components/service/secure_storage/backend/optee_storage/optee_storage_backend.h"
#include "components/service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h"

#define SP_NAME "storage.psp"

#define SP_OPTEE_STORAGE_UUID                                                  \
	{                                                                      \
		0x751bf801, 0x3dde, 0x4768,                                    \
		{                                                              \
			0xa5, 0x14, 0x0f, 0x10, 0xae, 0xed, 0x17, 0x90         \
		}                                                              \
	}

static TEE_Result sp_optee_storage_main(uint32_t session_id)
{
	struct sp_session *s = sp_get_session(session_id);
	struct thread_smc_args args = { 0 };
	uint16_t caller_id = 0;
	uint16_t own_id = 0;
	struct ffa_call_ep ffa_call_ep = { 0 };
	struct rpc_interface *secure_storage_iface = NULL;
	struct secure_storage_provider secure_storage_provider = { 0 };
	struct storage_backend *storage_backend = NULL;
	void *shmem_buf = NULL;
	size_t shmem_buf_size = 0;

	DMSG("SEL1 Storage SP init");

	pseudo_sp_ffa(s, &args);

	storage_backend = optee_storage_backend_init(TEE_STORAGE_PRIVATE_RPMB);
	secure_storage_iface = secure_storage_provider_init(
		&secure_storage_provider, storage_backend);
	ffa_call_ep_init(&ffa_call_ep, secure_storage_iface);

	while (true) {
		caller_id = ((args.a1 >> 16) & 0xffff);
		own_id = (args.a1 & 0xffff);

		/*
		 * Storage functions require variable to be accessible by the
		 * caller SEL0 SP (or TA).
		 */
		shmem_buf = ffa_call_ep_get_buffer(&ffa_call_ep, caller_id,
						   &shmem_buf_size);
		optee_storage_backend_assign_shared_buffer(
			storage_backend, shmem_buf, shmem_buf_size);

		ffa_call_ep_receive(&ffa_call_ep, &args, &args);

		args.a0 = FFA_MSG_SEND_DIRECT_RESP_32;
		args.a1 = ((own_id) << 16) | caller_id;
		args.a2 = 0x00;
		args.a3 = 0x00; /* RC = 0 */

		pseudo_sp_ffa(s, &args);
	}

	return TEE_SUCCESS;
}

pseudo_sp_register(.uuid = SP_OPTEE_STORAGE_UUID, .name = SP_NAME,
		   .invoke_command_entry_point = sp_optee_storage_main);
