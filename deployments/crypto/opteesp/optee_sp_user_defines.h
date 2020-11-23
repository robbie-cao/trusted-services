/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SP_HEADER_DEFINES_H
#define SP_HEADER_DEFINES_H

/* To get UUID definition */
#include "crypto_sp.h"

#define OPTEE_SP_UUID             CRYPTO_SP_UUID
#define OPTEE_SP_FLAGS			0

/* Provisioned stack size */
#define OPTEE_SP_STACK_SIZE			(64 * 1024)

/* Provisioned heap size */
#define OPTEE_SP_HEAP_SIZE			(480 * 1024)

#endif /* SP_HEADER_DEFINES_H */
