/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#ifndef MOCK_RPC_INTERFACE_H_
#define MOCK_RPC_INTERFACE_H_

#include "../endpoint/rpc_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

void mock_rpc_interface_init(void);

void expect_mock_rpc_interface_receive(struct rpc_interface *iface,
				       const struct call_req *req, rpc_status_t result);

rpc_status_t mock_rpc_interface_receive(struct rpc_interface *iface,
					struct call_req *req);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_RPC_INTERFACE_H_ */
