/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#ifndef OPTEE_SP_USER_DEFINES_H
#define OPTEE_SP_USER_DEFINES_H

#define OPTEE_SP_UUID \
	{0xa1baf155, 0x8876, 0x4695, \
		{0x8f, 0x7c, 0x54, 0x95, 0x5e, 0x8d, 0xb9, 0x74}}

#define OPTEE_SP_FLAGS				0

/* Provisioned stack size */
#define OPTEE_SP_STACK_SIZE			(64 * 1024)

#endif /* SP_HEADER_DEFINES_H */
