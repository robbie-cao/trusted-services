/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 */
#ifndef OPTEE_SP_USER_DEFINES_H
#define OPTEE_SP_USER_DEFINES_H

#define OPTEE_SP_UUID \
	{ 0x01109cf8, 0xe5ca, 0x446f, \
		{ 0x9b, 0x55, 0xf3, 0xcd, 0xc6, 0x51, 0x10, 0xc8 } }

#define OPTEE_SP_FLAGS			0

/* Provisioned stack size */
#define OPTEE_SP_STACK_SIZE			(64 * 1024)

#endif /* SP_HEADER_DEFINES_H */
