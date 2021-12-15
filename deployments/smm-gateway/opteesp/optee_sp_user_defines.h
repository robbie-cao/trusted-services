/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#ifndef SP_HEADER_DEFINES_H
#define SP_HEADER_DEFINES_H

#define OPTEE_SP_UUID \
	{0xed32d533, 0x99e6, 0x4209, \
		{ 0x9c, 0xc0, 0x2d, 0x72, 0xcd, 0xd9, 0x98, 0xa7 }}

#define OPTEE_SP_FLAGS				0

/* Provisioned stack size */
#define OPTEE_SP_STACK_SIZE			(64 * 1024)

#endif /* SP_HEADER_DEFINES_H */
