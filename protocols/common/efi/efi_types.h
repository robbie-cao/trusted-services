/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COMMON_EFI_TYPES_H
#define COMMON_EFI_TYPES_H

#include <stdint.h>

/**
 * Common EFI types
 */

/**
 * 128 bit buffer containing a unique identifier value.
 * Unless otherwise specified, aligned on a 64 bit boundary.
 */
typedef struct {
	uint32_t Data1;
	uint16_t Data2;
	uint16_t Data3;
	uint8_t  Data4[8];
} EFI_GUID;

/**
 * Header structure of messages in the MM communication buffer.
 */
typedef struct {
	EFI_GUID HeaderGuid;
	uint64_t MessageLength;
	uint8_t Data[1];
} EFI_MM_COMMUNICATE_HEADER;

/**
 * Size of the EFI MM_COMMUNICATE header without the data field.
 */
#define EFI_MM_COMMUNICATE_HEADER_SIZE	\
	(offsetof(EFI_MM_COMMUNICATE_HEADER, Data))

#endif /* COMMON_EFI_TYPES_H */
