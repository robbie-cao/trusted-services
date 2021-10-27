/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#ifndef SMM_VARIABLE_MM_SERVICE_H_
#define SMM_VARIABLE_MM_SERVICE_H_

#include "components/rpc/common/endpoint/rpc_interface.h"
#include "components/rpc/mm_communicate/endpoint/sp/mm_communicate_call_ep.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MM service interface implementation for parsing SMM variable requests and
 * forwarding them to an RPC interface.
 */

#define SMM_VARIABLE_GUID \
	{0xed32d533, 0x99e6, 0x4209, { 0x9c, 0xc0, 0x2d, 0x72, 0xcd, 0xd9, 0x98, 0xa7 }}

struct smm_variable_mm_service {
	struct mm_service_interface mm_service;
	struct rpc_interface *iface;
};

struct mm_service_interface *smm_variable_mm_service_init(struct smm_variable_mm_service *service,
							  struct rpc_interface *iface);

#ifdef __cplusplus
}
#endif

#endif /* SMM_VARIABLE_MM_SERVICE_H_ */
