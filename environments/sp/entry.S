/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 */

#include <asm.S>

#define	R_AARCH64_RELATIVE	1027

/**
 * The following code is responsible for setting the initial value of the stack
 * pointer and doing relocation on SP boot.
 */
FUNC __sp_entry, :
	/* Use __stack_end linker symbol to set the load relative stack address. */
	adrp	x4, __stack_end
	add	x4, x4, :lo12:__stack_end
	mov	sp, x4

	/*
	 * X4 = load address
	 * X5 = relocation table start
	 * X6 = relocation table end
	 */
	adr	x4, __sp_entry
	adrp	x5, __rela_start
	add	x5, x5, :lo12:__rela_start
	adrp	x6, __rela_end
	add	x6, x6, :lo12:__rela_end

	/* Iterating through relocation entries */
	cmp	x5, x6
	beq	2f

	/*
	 * Loading relocation entry
	 * X7 = r_offset
	 * X8 = r_info
	 * X9 = r_addend
	 */
1:	ldp	x7, x8, [x5], #16
	ldr	x9, [x5], #8

	/* Only R_AARCH64_RELATIVE type is supported */
	cmp	w8, #R_AARCH64_RELATIVE
	bne	3f	/* Error */

	/*
	 * Apply relative adjustment on address
	 * *(load_address + r_offset) = load_address + r_addend
	 */
	add	x9, x9, x4
	str	x9, [x7, x4]

	cmp	x5, x6
	bne	1b

2:
	b	_sp_entry

3:
	adr	X0,	error_invalid_relocation
	bl	trace_puts
	b	.

	.align 8
error_invalid_relocation:
	.asciz "Only R_AARCH64_RELATIVE type relocation is supported"
	.align 8
END_FUNC __sp_entry
