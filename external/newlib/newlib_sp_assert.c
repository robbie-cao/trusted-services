// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include <assert.h>
#include "compiler.h"
#include "trace.h"

/*
 * The generic trace function called on assert fail.
 */
void __noreturn __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
#if TRACE_LEVEL >= TRACE_LEVEL_ERROR
	trace_printf(func, line, TRACE_LEVEL_ERROR, "assertion %s failed", failedexpr);
#endif /* TRACE_LEVEL */

	while (1)
		;
}
