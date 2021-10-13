/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TS_SMM_VARIABLE_PARAMETERS_H
#define TS_SMM_VARIABLE_PARAMETERS_H

#include <protocols/common/efi/efi_types.h>

/**
 * C/C++ definition of smm_variable service parameters
 *
 * These defines are aligned to the SMM Variable definitions from EDK2. These versions
 * of these defines are maintained in the TS project to avoid a mandatory dependency
 * on the EDK2 project.
 */

/**
 * Variable attributes
 */
#define	EFI_VARIABLE_NON_VOLATILE							(0x00000001)
#define	EFI_VARIABLE_BOOTSERVICE_ACCESS						(0x00000002)
#define	EFI_VARIABLE_RUNTIME_ACCESS							(0x00000004)
#define	EFI_VARIABLE_HARDWARE_ERROR_RECORD					(0x00000008)
#define	EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS				(0x00000010)
#define	EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS	(0x00000020)
#define	EFI_VARIABLE_APPEND_WRITE							(0x00000040)
#define	EFI_VARIABLE_MASK \
	(EFI_VARIABLE_NON_VOLATILE | \
	 EFI_VARIABLE_BOOTSERVICE_ACCESS | \
	 EFI_VARIABLE_RUNTIME_ACCESS | \
	 EFI_VARIABLE_HARDWARE_ERROR_RECORD | \
	 EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS | \
	 EFI_VARIABLE_APPEND_WRITE)

/**
 * Parameter structure for SetVariable and GetVariable.
 */
typedef struct {
	EFI_GUID		Guid;
	uint64_t		DataSize;
	uint64_t		NameSize;
	uint32_t		Attributes;
	int16_t			Name[1];
} SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE;

#define SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_NAME_OFFSET \
	offsetof(SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE, Name)

#define SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_DATA_OFFSET(s) \
	(offsetof(SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE, Name) + s->NameSize)

#define SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_TOTAL_SIZE(s) \
	(offsetof(SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE, Name) + s->NameSize + s->DataSize)

#define SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_SIZE(name_size, data_size) \
	(offsetof(SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE, Name) + name_size + data_size)

/**
 * Parameter structure for GetNextVariableName.
 */
typedef struct {
	EFI_GUID		Guid;
	uint64_t		NameSize;
	int16_t			Name[1];
} SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME;

#define SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_NAME_OFFSET \
	offsetof(SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME, Name)

#define SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_TOTAL_SIZE(s) \
	(offsetof(SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME, Name) + s->NameSize)

#define SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_SIZE(name_size) \
	(offsetof(SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME, Name) + name_size)

/**
 * Parameter structure for QueryVariableInfo.
 */
typedef struct {
	uint64_t		MaximumVariableStorageSize;
	uint64_t		RemainingVariableStorageSize;
	uint64_t		MaximumVariableSize;
	uint32_t		Attributes;
} SMM_VARIABLE_COMMUNICATE_QUERY_VARIABLE_INFO;


#endif /* TS_SMM_VARIABLE_PARAMETERS_H */
