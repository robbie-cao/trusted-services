// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#include "compiler.h"
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

/* Allocating heap area */
#ifndef SP_HEAP_SIZE
#error "SP_HEAP_SIZE is undefined, please define it in the build system"
#endif

static uint8_t sp_heap[SP_HEAP_SIZE] __aligned(16);
static uint8_t *program_break = sp_heap;

/**
 * Basic sbrk implementation which increases the program break through the
 * sp_heap buffer.
 */
void *_sbrk(ptrdiff_t incr)
{
	uint8_t *previous_break = program_break;
	uint8_t *new_break = program_break + incr;

	if ((new_break < sp_heap) || (new_break > (sp_heap + sizeof(sp_heap)))) {
		errno = ENOMEM;
		return (void *)(uintptr_t) -1;
	}

	program_break += incr;

	return (void *) previous_break;
}
