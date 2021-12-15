// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#include "trace.h"
#include "ffa_internal_api.h"

#if TRACE_LEVEL >= TRACE_LEVEL_ERROR

void trace_puts(const char *str)
{
	struct ffa_params resp;

	ffa_svc(0xdeadbeef, (uintptr_t)str, 0, 0, 0, 0, 0, 0, &resp);
}

#endif  /* TRACE_LEVEL >= TRACE_LEVEL_ERROR */
