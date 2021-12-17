/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#ifndef ASSERT_FAIL_HANDLER_H_
#define ASSERT_FAIL_HANDLER_H_

#include "compiler.h"

/*
 * Generic assert fail handler function definition. Should be implemented by the environment.
 */
void __noreturn assert_fail_handler(const char *file, int line,
				    const char *func, const char *failedexpr);

#endif /* ASSERT_FAIL_HANDLER_H_ */
