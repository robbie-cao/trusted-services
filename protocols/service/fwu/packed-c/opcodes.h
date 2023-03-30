/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWU_PROTO_OPCODES_H
#define FWU_PROTO_OPCODES_H

/**
 * Service-level opcodes
 */
#define TS_FWU_OPCODE_BASE	      (0x10)
#define TS_FWU_OPCODE_BEGIN_STAGING   (TS_FWU_OPCODE_BASE + 0)
#define TS_FWU_OPCODE_END_STAGING     (TS_FWU_OPCODE_BASE + 1)
#define TS_FWU_OPCODE_CANCEL_STAGING  (TS_FWU_OPCODE_BASE + 2)
#define TS_FWU_OPCODE_OPEN	      (TS_FWU_OPCODE_BASE + 3)
#define TS_FWU_OPCODE_WRITE_STREAM    (TS_FWU_OPCODE_BASE + 4)
#define TS_FWU_OPCODE_READ_STREAM     (TS_FWU_OPCODE_BASE + 5)
#define TS_FWU_OPCODE_COMMIT	      (TS_FWU_OPCODE_BASE + 6)
#define TS_FWU_OPCODE_ACCEPT_IMAGE    (TS_FWU_OPCODE_BASE + 7)
#define TS_FWU_OPCODE_SELECT_PREVIOUS (TS_FWU_OPCODE_BASE + 8)

#endif /* FWU_PROTO_OPCODES_H */
