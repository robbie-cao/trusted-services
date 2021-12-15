/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 */

#ifndef OPTEE_SP_USER_DEFINES_H
#define OPTEE_SP_USER_DEFINES_H

#define OPTEE_SP_UUID \
	{ 0xdc1eef48, 0xb17a, 0x4ccf, \
		{ 0xac, 0x8b, 0xdf, 0xcf, 0xf7, 0x71, 0x1b, 0x14 } }

#define OPTEE_SP_FLAGS			0

/* Provisioned stack size */
#define OPTEE_SP_STACK_SIZE			(64 * 1024)

#endif /* SP_HEADER_DEFINES_H */
