/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#ifndef OPTEE_SP_USER_DEFINES_H
#define OPTEE_SP_USER_DEFINES_H

#define OPTEE_SP_UUID \
	{0x33c75baf, 0xac6a, 0x4fe4, \
		{0x8a, 0xc7, 0xe9, 0x90, 0x9b, 0xee, 0x2d, 0x17}}

#define OPTEE_SP_FLAGS				0

/* Provisioned stack size */
#define OPTEE_SP_STACK_SIZE			(64 * 1024)

#endif /* SP_HEADER_DEFINES_H */
