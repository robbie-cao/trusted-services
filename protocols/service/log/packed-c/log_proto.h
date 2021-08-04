/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#ifndef LOG_PROTO_H
#define LOG_PROTO_H

#include "compiler.h"
#include <stdint.h>

struct __packed log_request {
	uint64_t msg_length;
	char msg[];
};

/* Opcodes */
#define TS_LOG_OPCODE_PUTS (0u)

#endif /* LOG_PROTO_H */
