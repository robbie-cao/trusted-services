/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ffa_internal_api.h"
#include "optee_sp_internal_api.h"
#include "sp_api.h"

void __noreturn optee_sp_entry(uintptr_t a0, uintptr_t a1, uintptr_t a2,
			 uintptr_t a3)
{
	(void)a1;
	(void)a2;
	(void)a3;

	sp_main((struct ffa_init_info *)a0);
}

void optee_sp_log_puts(const char *str)
{
	(void)str;
}

void __noreturn optee_sp_panic(void)
{
	while (1)
		;
}
