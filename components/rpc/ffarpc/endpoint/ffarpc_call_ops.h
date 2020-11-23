/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FFA_CALL_OPS_H
#define FFA_CALL_OPS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Common opcodes used by the FFA based RPC layer for management operations */
#define FFA_CALL_OPCODE_BASE		(0x10)
#define FFA_CALL_OPCODE_SHARE_BUF	(FFA_CALL_OPCODE_BASE + 0)
#define FFA_CALL_OPCODE_UNSHARE_BUF	(FFA_CALL_OPCODE_BASE + 1)
#define FFA_CALL_OPCODE_LIMIT		(FFA_CALL_OPCODE_BASE + 2)

#define FFA_CALL_OPCODE_IS_MGMT(opcode) \
	((opcode >= FFA_CALL_OPCODE_BASE) && (opcode < FFA_CALL_OPCODE_LIMIT))

#ifdef __cplusplus
}
#endif

#endif /* FFA_CALL_OPS_H */
