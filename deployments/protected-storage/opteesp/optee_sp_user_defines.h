/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 */

#ifndef OPTEE_SP_USER_DEFINES_H
#define OPTEE_SP_USER_DEFINES_H

#define OPTEE_SP_UUID \
	{ 0x751bf801, 0x3dde, 0x4768, \
		{ 0xa5, 0x14, 0x0f, 0x10, 0xae, 0xed, 0x17, 0x90 } }

#define OPTEE_SP_FLAGS			0

/* Provisioned stack size */
#define OPTEE_SP_STACK_SIZE			(64 * 1024)

#endif /* SP_HEADER_DEFINES_H */
