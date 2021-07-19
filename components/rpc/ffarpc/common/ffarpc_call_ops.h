/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FFA_CALL_OPS_H
#define FFA_CALL_OPS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Common opcodes used by the FFA based RPC layer for management operations */
enum
{
	FFA_CALL_OPCODE_SHARE_BUF		= 0,
	FFA_CALL_OPCODE_UNSHARE_BUF		= 1,
	FFA_CALL_OPCODE_LIMIT
};

/* Interface ID for FFA management interface */
#define FFA_CALL_MGMT_IFACE_ID		(0x1000)

#ifdef __cplusplus
}
#endif

#endif /* FFA_CALL_OPS_H */
