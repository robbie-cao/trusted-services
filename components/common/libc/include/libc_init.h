/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#ifndef LIBC_INIT_H_
#define LIBC_INIT_H_

/*
 * Generic libc init function. Implemented by the newlib external, should be called by the
 * environment on boot.
 */
void libc_init(void);

#endif /* LIBC_INIT_H_ */
