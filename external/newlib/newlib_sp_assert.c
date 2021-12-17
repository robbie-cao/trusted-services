// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#include "assert_fail_handler.h"
#include "compiler.h"
#include <assert.h>

/*
 * This function implements newlib's assert fail handler function by calling the
 * generic assert fail handler function that should be implemented by the
 * environment.
 */
void __noreturn __assert_func(const char *file, int line, const char *func,
			      const char *failedexpr)
{
	assert_fail_handler(file, line, func, failedexpr);
	while (1)
		;
}
