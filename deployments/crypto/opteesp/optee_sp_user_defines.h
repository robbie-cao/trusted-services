/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 */

#ifndef OPTEE_SP_USER_DEFINES_H
#define OPTEE_SP_USER_DEFINES_H

#define OPTEE_SP_UUID \
	{0xd9df52d5, 0x16a2, 0x4bb2, \
		{0x9a, 0xa4, 0xd2, 0x6d, 0x3b, 0x84, 0xe8, 0xc0}}

#define OPTEE_SP_FLAGS			0

/* Provisioned stack size */
#define OPTEE_SP_STACK_SIZE			(64 * 1024)

#endif /* SP_HEADER_DEFINES_H */
